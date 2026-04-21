module;

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <unknwn.h> // IUnknown 정의를 위해 필수
#include <inspectable.h>
#include <windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <iostream>


// [필수] Interop 헤더 (Native HWND 연결용)
#include <Windows.Graphics.Capture.Interop.h>
#include <Windows.Graphics.DirectX.Direct3D11.interop.h>

// [필수] WinRT 프로젝션 헤더 (.h와 .hpp 혼용 권장)
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>

#include <opencv2/opencv.hpp>

// 링커 설정
#pragma comment(lib, "windowsapp.lib")
#pragma comment(lib, "d3d11.lib")

#include <ShellScalingApi.h> // SetProcessDpiAwareness 호출용  (디스플레이 배율 무시)
#pragma comment(lib, "Shcore.lib") // 라이브러리 링크

#include <atomic>
#include <chrono>


module RenderManager;

// --- 컴파일러 에러 C2039 해결을 위한 강제 정의 ---
// 이 UUID와 구조는 Windows SDK 10.0.19041.0 이상에서 공식적으로 정의된 내용입니다.
DECLARE_INTERFACE_IID_(ICaptureInteropFix, IUnknown, "3628E81B-3CAC-4C60-B7F4-23CE0E0C3356")
{
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) = 0;
    STDMETHOD_(ULONG, AddRef)(void) = 0;
    STDMETHOD_(ULONG, Release)(void) = 0;

    // SDK 헤더에서 못 찾던 그 함수를 직접 명시합니다.
    STDMETHOD(CreateFreeThreadedEx)(
        HWND window,
        REFIID riid,
        void** result) = 0;
};


enum CropColorType : int{
    BGRA = 87,
    RGBA = 28
};


class CropEngineV1 {
private:

    ID3D11Device* m_d3dDevice = nullptr;
    ID3D11DeviceContext* m_d3dContext = nullptr;
    
    ID3D11Texture2D* m_stagingTexture = nullptr;
    winrt::Windows::Graphics::SizeInt32 m_lastSize{ 0, 0 };

    // WinRT 객체들
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_winrtDevice{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ nullptr };

    std::atomic<bool> m_isRunning{ false }; // 종료 플래그
    winrt::event_token m_frameArrivedToken;

    
public:
    cv::Mat m_latestFrame; // 공유 버퍼
    std::mutex m_frameMutex; // 멀티스레드 충돌 방지용  
    std::chrono::steady_clock::time_point m_lastProcessTime;
    int m_targetFPS = 10; // 원하는 FPS 설정
    std::atomic<bool> m_isFrameNew{ false };  // 프레임이 갱신되었을때만 imshow호출
    int m_CropColorType = CropColorType::BGRA;

    CropEngineV1() : m_stagingTexture(nullptr) {
        // 1. D3D11 장치 생성
        D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            nullptr, 0, D3D11_SDK_VERSION, &m_d3dDevice, nullptr, &m_d3dContext);

        // 2. WinRT 디바이스로 변환 (직접 호출 방식)
        winrt::com_ptr<IDXGIDevice> dxgiDevice;
        m_d3dDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice.put()));

        winrt::com_ptr<IInspectable> inspectable;
        // 글로벌 함수 직접 호출 (네임스페이스 에러 회피)
        ::CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), inspectable.put());
        m_winrtDevice = inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();
    }

    void changeCropColorType(){
        // change color type bgra <-> rgba for opencv or wxWidget kinds gui
        if (m_CropColorType == CropColorType::BGRA) {
            m_CropColorType = CropColorType::RGBA;
            return;
        }else if(m_CropColorType == CropColorType::RGBA){
            m_CropColorType = CropColorType::BGRA;
            return;
        }
    }

    void start(HWND hwnd) {
        if (!IsWindow(hwnd)) {
            std::cout << "the hwnd is not indicate Window (79)" << std::endl;
            return;
        }
        m_isRunning = true;
        // 3. HWND -> CaptureItem (Interop 팩토리 직접 사용)
        auto interop_factory = winrt::get_activation_factory<
            winrt::Windows::Graphics::Capture::GraphicsCaptureItem, 
            // IGraphicsCaptureItemInterop>();
            ICaptureInteropFix>();

        winrt::Windows::Graphics::Capture::GraphicsCaptureItem item{ nullptr };
        // 멤버 함수 호출 에러 방지를 위해 명시적 팩토리 호출
        interop_factory->CreateFreeThreadedEx(
            hwnd, 
            winrt::guid_of<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>(), 
            winrt::put_abi(item));

        // 4. FramePool 생성 (열거형 미인식 대비 숫자 87 사용)
        // 87 = DirectXPixelFormat::B8G8R8A8UintNormalized
        auto pixelFormat = static_cast<winrt::Windows::Graphics::DirectX::DirectXPixelFormat>(m_CropColorType);

        m_framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(
            m_winrtDevice, 
            pixelFormat, 
            2, 
            item.Size());

        // 5. 세션 시작 및 이벤트 핸들러
        m_session = m_framePool.CreateCaptureSession(item);

        m_frameArrivedToken = m_framePool.FrameArrived([this](auto const& sender, auto const&) {
            auto frame = sender.TryGetNextFrame();
            if (frame) {
                // 실시간 프레임 수신 확인용 로그
                
                processFrame(frame);
                frame.Close();
            }
        });
        

        m_session.StartCapture();
    }

    //해상도변경 반영 미완성이슈
    void processFrame_prev(winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame const& frame) {
        if (!m_isRunning) return;
        // 1. 프레임으로부터 DXGI 표면 가져오기
        auto surface = frame.Surface();
        auto access = surface.as<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
        
        winrt::com_ptr<ID3D11Texture2D> gpuTexture;
        access->GetInterface(IID_PPV_ARGS(gpuTexture.put()));

        // 2. 텍스처 정보 확인
        D3D11_TEXTURE2D_DESC desc;
        gpuTexture->GetDesc(&desc);

        // 3. CPU에서 읽을 수 있는 임시(Staging) 텍스처 생성 (최초 1회 또는 크기 변경시)
        if (m_stagingTexture == nullptr || 
            m_lastSize.Width != desc.Width || 
            m_lastSize.Height != desc.Height
            ) {

            m_lastSize.Width = desc.Width;
            m_lastSize.Height = desc.Height;


            D3D11_TEXTURE2D_DESC stagingDesc = desc;
            stagingDesc.Usage = D3D11_USAGE_STAGING;
            stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            stagingDesc.BindFlags = 0;
            stagingDesc.MiscFlags = 0;
            m_d3dDevice->CreateTexture2D(&stagingDesc, nullptr, &m_stagingTexture);

        }

        // 4. GPU 데이터를 Staging 텍스처로 복사
        m_d3dContext->CopyResource(m_stagingTexture, gpuTexture.get());

        // 5. 메모리 매핑하여 데이터 읽기
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(m_d3dContext->Map(m_stagingTexture, 0, D3D11_MAP_READ, 0, &mapped))) {
            
            // 6. OpenCV Mat 생성 (B8G8R8A8 포맷이므로 CV_8UC4)
            cv::Mat mat(desc.Height, desc.Width, CV_8UC4, mapped.pData, mapped.RowPitch);

            {
                std::lock_guard<std::mutex> lock(m_frameMutex);
                // 데이터를 복사만 하고 imshow는 하지 않습니다!
                mat.copyTo(m_latestFrame); 
            }
        
            m_d3dContext->Unmap(m_stagingTexture, 0);
        }
    }

    void processFrame(winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame const& frame) {
        if (!m_isRunning) return;

        // 현재 시간 계산
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastProcessTime).count();

        // 설정한 FPS 기준 시간(ms)보다 빠르게 호출되면 즉시 리턴
        // 예: 30FPS = 약 33ms 마다 한 번씩 처리
        if (elapsed < (1000 / m_targetFPS)) {
            return; 
        }
        m_lastProcessTime = now;

        // std::cout << "Captured Frame Size below: " << frame.ContentSize().Width << "x" << frame.ContentSize().Height << std::endl;
        // 1. 프레임의 실제 콘텐츠 크기 가져오기 (해상도 감지의 기준)
        auto contentSize = frame.ContentSize(); 

        // 2. [해상도 변경 처리] 이전 크기와 다르다면 리소스 재생성
        if (contentSize.Width != m_lastSize.Width || contentSize.Height != m_lastSize.Height) {
            
            m_lastSize = contentSize; // 새로운 크기 저장

            // Staging Texture(CPU 복사용) 재생성
            if (m_stagingTexture) {
                m_stagingTexture->Release();
                m_stagingTexture = nullptr;
            }

            D3D11_TEXTURE2D_DESC stagingDesc = {};
            stagingDesc.Width = contentSize.Width;
            stagingDesc.Height = contentSize.Height;
            stagingDesc.MipLevels = 1;
            stagingDesc.ArraySize = 1;
            if (m_CropColorType == CropColorType::BGRA){
                    stagingDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 일반적인 WGC 포맷, BGRA 87
                }
            else {
                stagingDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 일반적인 WGC 포맷, RGBA 28
            }
            stagingDesc.SampleDesc.Count = 1;
            stagingDesc.Usage = D3D11_USAGE_STAGING;
            stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

            m_d3dDevice->CreateTexture2D(&stagingDesc, nullptr, &m_stagingTexture);

            // [핵심] FramePool에게 새로운 해상도로 프레임을 보내라고 명령
            m_framePool.Recreate(
                m_winrtDevice, 
                static_cast<winrt::Windows::Graphics::DirectX::DirectXPixelFormat>(m_CropColorType), // B8G8R8A8_UNORM = 87, R8G8B8A8_UNORM = 28
                2, 
                m_lastSize);

            std::cout << "[Resizing] New Size: " << m_lastSize.Width << "x" << m_lastSize.Height << std::endl;
            return; // 크기가 바뀐 직후 프레임은 버퍼 크기가 불일치하므로 스킵
        }

        // 3. GPU 텍스처 접근
        auto surface = frame.Surface();
        auto access = surface.as<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
        winrt::com_ptr<ID3D11Texture2D> gpuTexture;
        access->GetInterface(IID_PPV_ARGS(gpuTexture.put()));

        // 4. GPU 데이터를 CPU 가독 텍스처(Staging)로 복사
        if (m_stagingTexture && gpuTexture) {
            m_d3dContext->CopyResource(m_stagingTexture, gpuTexture.get());

            D3D11_MAPPED_SUBRESOURCE mapped;
            if (SUCCEEDED(m_d3dContext->Map(m_stagingTexture, 0, D3D11_MAP_READ, 0, &mapped))) {
                
                // 5. OpenCV Mat 생성 (현재 해상도 기준)
                cv::Mat currentMat(m_lastSize.Height, m_lastSize.Width, CV_8UC4, mapped.pData, mapped.RowPitch);
                
                {
                    // 6. 메인 스레드와 공유하는 버퍼에 안전하게 복사 (크기 자동 조정 포함)
                    std::lock_guard<std::mutex> lock(m_frameMutex);
                    currentMat.copyTo(m_latestFrame);
                    m_isFrameNew = true; 
                }

                m_d3dContext->Unmap(m_stagingTexture, 0);
            }
        }
    
    
}

    void stop() {
        m_isRunning = false;
        
        // 1. 세션 종료 (프레임 유입 차단)
        if (m_session) {
            m_session.Close();
            m_session = nullptr;
        }

        // 2. 프레임 풀 종료 및 이벤트 연결 해제
        if (m_framePool) {
            m_framePool.FrameArrived(m_frameArrivedToken);
            m_framePool.Close();
            m_framePool = nullptr;
        }
        

        // 3. DXGI/D3D 자원 해제
        if (m_stagingTexture) {
            m_stagingTexture->Release();
            m_stagingTexture = nullptr;
        }
        
        std::cout << "Engine Stopped Successfully." << std::endl;
    }

    // 윈도우의 실제 시각적 좌표와 Win32 좌표 사이의 오차 계산
    POINT GetCoordinateOffset(HWND hWnd) {
        RECT windowRect, extendedRect;
        
        // 전체 윈도우 좌표 (투명 테두리 포함)
        GetWindowRect(hWnd, &windowRect);
        
        // 실제 눈에 보이는 윈도우 좌표 (DWM 기준)
        DwmGetWindowAttribute(hWnd, DWMWA_EXTENDED_FRAME_BOUNDS, &extendedRect, sizeof(RECT));
        
        // 오차 반환
        return {
            extendedRect.left - windowRect.left,
            extendedRect.top - windowRect.top
        };
    }

    // 윈도우 핸들(hWnd)을 인자로 받아 클라이언트 시작 오프셋을 계산
    POINT GetClientOffset(HWND hWnd) {
        RECT rectWindow;
        GetWindowRect(hWnd, &rectWindow); // 창 전체의 화면 좌표

        POINT ptClientStart = { 0, 0 };
        ClientToScreen(hWnd, &ptClientStart); // 클라이언트 (0,0)의 화면 좌표, 메뉴바가 포함되는것은 정상이며
                                              // 그림판의 그림이 그려지는 부분만 캡쳐하려면 자식윈도우 에서 찾아야 함.

        // 전체 창 좌표와 클라이언트 시작 좌표의 차이 계산
        POINT offset;
        offset.x = ptClientStart.x - rectWindow.left; // 보통 테두리 두께 (약 8~10px)
        offset.y = ptClientStart.y - rectWindow.top;  // 보통 타이틀바 + 메뉴바 높이

        //PostMessage용 좌표 = 캡처 이미지 좌표 + DWM 오차(10,0) - 클라이언트 영역 시작 오차
        // DWM 오차(10,0) 는 윈 10,11의 표준인듯

        return offset;
    }

    ~CropEngineV1() {
        if (m_stagingTexture) {
            m_stagingTexture->Release();
            m_stagingTexture = nullptr;
        }

        if (m_session) m_session.Close();
        if (m_framePool) m_framePool.Close();
        if (m_d3dDevice) m_d3dDevice->Release();
    }
};



int main() {
    // 프로세스를 DPI aware 상태로 설정 (디스플레이배율 적용하지 않도록)
    // 윈도우 10/11 환경에서 가장 권장되는 방식 (Per Monitor v2 Aware)
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    setlocale(LC_ALL,"");  // 한글출력설정 제대로 (wprintf)
    // WinRT 아파트먼트 초기화 필수
    winrt::init_apartment();

    CropEngineV1 engine;
    
    auto win_name = std::wstring(L"제목 없음 - 그림판");

    HWND target = FindWindowW(nullptr, win_name.c_str());

    if (target){
        cv::namedWindow("Capture View", cv::WINDOW_AUTOSIZE);
        engine.start(target);
        // engine.stop();
        // engine.changeCropColorType();
        // engine.start(target);
        
        // 이전 프레임의 크기를 저장할 변수
        int lastWidth = 0; 
        int lastHeight = 0;
        
        while (true) {
            {
                std::lock_guard<std::mutex> lock(engine.m_frameMutex);
                if (!engine.m_isFrameNew) continue; // no redraw if the frame isn't new one.
                
                if (!engine.m_latestFrame.empty()) {
                    
                    int currentWidth = engine.m_latestFrame.cols;
                    int currentHeight = engine.m_latestFrame.rows;
                    
                    // [핵심] 해상도가 변했는지 체크
                    if (currentWidth != lastWidth || currentHeight != lastHeight) {
                        // 1. 창의 크기 조절 모드를 일시적으로 해제했다가 다시 설정 (일종의 리셋)
                        cv::destroyWindow("Capture View");
                        cv::namedWindow("Capture View", cv::WINDOW_AUTOSIZE);
                        std::cout << "창크기변경" << lastWidth << lastHeight<< std::endl;
                        lastWidth = currentWidth;
                        lastHeight = currentHeight;
                        std::cout << "창크기변경" << currentWidth << currentHeight<< std::endl;
                        std::cout << "해상도 변경 감지: " << lastWidth << "x" << lastHeight << std::endl;
                        POINT diff_P = engine.GetCoordinateOffset(target);
                        std::cout << "윈도우창들간의 크기차이 (x,y):" << diff_P.x << ", " << diff_P.y << std::endl;
                        POINT diff_P2 =engine.GetClientOffset(target);
                        std::cout << "클라이언트 오프셋(x,y):" << diff_P2.x << ", " << diff_P2.y << std::endl;
                        cv::rectangle(engine.m_latestFrame, cv::Rect(diff_P2.x, diff_P2.y, lastWidth-diff_P2.x, lastHeight-diff_P2.y),cv::Scalar(0,0,255));
                    }

                    cv::imshow("Capture View", engine.m_latestFrame);
                }

                engine.m_isFrameNew = false; // frame 소모 완료
            }

            if (cv::waitKey(17) == 27) break;
        }

        engine.stop();
        cv::destroyAllWindows();

    } else {
        // std::wcout << "Target Window "<< win_name << "not found." << std::endl;
        wprintf(L"the window name : %s not found", win_name.c_str());
        // printf("the window %s not found",win_name);
    }
   
    return 0;
}