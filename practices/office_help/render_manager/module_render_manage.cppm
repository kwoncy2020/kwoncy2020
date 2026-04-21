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


export module RenderManager;

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

export enum CropColorType : int{
    BGRA = 87,
    RGBA = 28
};

export CropEngineV1 {
    private:
    ID3D11Device* m_d3dDevice = nullptr;
    ID3D11DeviceContext* m_d3dContext = nullptr;
    
    ID3D11Texture2D* m_stagingTexture = nullptr;
    winrt::Windows::Graphics::SizeInt32 m_lastSize{ 0, 0 };

    // WinRT 객체들
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_winrtDevice{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_item{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker m_frameArrivedRevoker;
    
    // 멤버 변수
    std::atomic<bool> m_isRunning{ false }; // 종료 플래그
    winrt::event_token m_frameArrivedToken;

    public:
        CropEngineV1();
        ~CropEngineV1();

        cv::Mat m_latestFrame; // 공유 버퍼
        std::mutex m_frameMutex; // 멀티스레드 충돌 방지용  
        std::chrono::steady_clock::time_point m_lastProcessTime;
        int m_targetFPS = 10; // 원하는 FPS 설정
        std::atomic<bool> m_isFrameNew{ false };  // 프레임이 갱신되었을때만 imshow호출
        int m_CropColorType = CropColorType::BGRA;

        void changeCropColorType();
        void start(HWND hwnd);
        void stop();
        void processFrame(winrt::Windows::Graphics::Capture::Direct3D11CaptureFrame const& frame);
        POINT GetCoordinateOffset(HWND hWnd);
        POINT GetClientOffset(HWND hWnd);

};
