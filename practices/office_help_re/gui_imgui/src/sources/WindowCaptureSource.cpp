#include "sources/WindowCaptureSource.h"
#include <iostream>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <dwmapi.h>
#include <shellscalingapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shcore.lib")
#endif

WindowCaptureSource::WindowCaptureSource() 
    : m_captureType(CaptureType::Desktop), m_windowHandle(nullptr),
      m_captureSize(1920, 1080), m_fps(30.0), m_includeBorders(true),
      m_includeCursor(false) {
    updateAvailableWindows();
}

WindowCaptureSource::~WindowCaptureSource() {
    close();
}

bool WindowCaptureSource::open(const std::string& source) {
    m_lastError.clear();
    
    // 소스 문자열 파싱
    if (source == "desktop") {
        return openDesktop();
    } else if (source.find("window:") == 0) {
        std::string title = source.substr(7); // "window:" 제거
        return openWindow(title);
    } else if (source.find("region:") == 0) {
        // region:x,y,width,height 형식 파싱
        std::string regionStr = source.substr(7);
        // 간단한 파싱 (실제로는 더 정교한 파싱 필요)
        return openDesktop(); // 임시로 데스크톱으로
    } else {
        // 윈도우 제목으로 간주
        return openWindow(source);
    }
}

bool WindowCaptureSource::close() {
    m_isOpen = false;
    m_currentFrameMat.release();
    
    std::cout << "Closed window capture source" << std::endl;
    return true;
}

bool WindowCaptureSource::isOpen() const {
    return m_isOpen;
}

bool WindowCaptureSource::readNextFrame(cv::Mat& frame) {
    if (!isOpen()) {
        m_lastError = "Window capture source is not open";
        return false;
    }
    
    // FPS 제어
    if (!shouldCaptureNextFrame()) {
        return false;
    }
    
    bool success = false;
    
    switch (m_captureType) {
        case CaptureType::Window:
            success = captureWindowFrame(m_windowHandle, frame);
            break;
            
        case CaptureType::Desktop:
            success = captureDesktopFrame(frame);
            break;
            
        case CaptureType::Region:
            success = captureRegionFrame(m_captureRegion, frame);
            break;
            
        default:
            m_lastError = "Unknown capture type";
            return false;
    }
    
    if (success) {
        m_currentFrameMat = frame.clone();
        m_lastCaptureTime = std::chrono::steady_clock::now();
    }
    
    return success;
}

bool WindowCaptureSource::readFrameAt(int frameIndex, cv::Mat& frame) {
    // 윈도우 캡처는 라이브 소스이므로 특정 프레임 탐색 불가
    m_lastError = "Window capture source does not support frame seeking";
    return false;
}

bool WindowCaptureSource::seekToFrame(int frameIndex) {
    m_lastError = "Window capture source does not support frame seeking";
    return false;
}

bool WindowCaptureSource::seekToTime(double timeInSeconds) {
    m_lastError = "Window capture source does not support time seeking";
    return false;
}

DataSourceInfo WindowCaptureSource::getSourceInfo() const {
    DataSourceInfo info;
    
    switch (m_captureType) {
        case CaptureType::Window:
            info.sourceType = "Window Capture";
            info.sourcePath = "Window: " + m_windowTitle;
            break;
        case CaptureType::Desktop:
            info.sourceType = "Desktop Capture";
            info.sourcePath = "Desktop";
            break;
        case CaptureType::Region:
            info.sourceType = "Region Capture";
            info.sourcePath = "Region: " + std::to_string(m_captureRegion.x) + "," + 
                            std::to_string(m_captureRegion.y) + "," +
                            std::to_string(m_captureRegion.width) + "," +
                            std::to_string(m_captureRegion.height);
            break;
    }
    
    info.isLive = true;
    info.width = m_captureSize.width;
    info.height = m_captureSize.height;
    info.channels = 4; // 대부분의 화면 캡처는 알파 채널 포함
    info.fps = m_fps;
    info.totalFrames = -1; // 무제한 프레임
    info.duration = -1.0; // 무제한 시간
    info.currentFrame = -1;
    info.currentTime = -1.0;
    info.canSeek = false;
    info.canPause = false;
    info.canRewind = false;
    info.supportsRealTime = true;
    
    return info;
}

std::string WindowCaptureSource::getLastError() const {
    return m_lastError;
}

bool WindowCaptureSource::isLive() const {
    return true;
}

int WindowCaptureSource::getCurrentFrameIndex() const {
    return -1;
}

double WindowCaptureSource::getCurrentTime() const {
    return -1.0;
}

bool WindowCaptureSource::hasMoreFrames() const {
    return isOpen();
}

void WindowCaptureSource::reset() {
    // 윈도우 캡처는 리셋 의미가 없음
}

// 윈도우 캡처 특화 메서드

bool WindowCaptureSource::openWindow(void* windowHandle) {
    if (!windowHandle) {
        m_lastError = "Invalid window handle";
        return false;
    }
    
    m_windowHandle = windowHandle;
    m_captureType = CaptureType::Window;
    m_windowTitle = getWindowTitle(windowHandle);
    
    // 윈도우 크기 가져오기
    cv::Rect windowRect = getWindowRect(windowHandle);
    m_captureSize = cv::Size(windowRect.width, windowRect.height);
    
    m_isOpen = true;
    m_lastCaptureTime = std::chrono::steady_clock::now();
    
    std::cout << "Opened window capture: " << m_windowTitle 
              << " (" << m_captureSize.width << "x" << m_captureSize.height << ")" << std::endl;
    
    return true;
}

bool WindowCaptureSource::openWindow(const std::string& windowTitle) {
    void* handle = findWindowHandleByTitle(windowTitle);
    if (!handle) {
        m_lastError = "Window not found: " + windowTitle;
        return false;
    }
    
    return openWindow(handle);
}

bool WindowCaptureSource::openDesktop() {
    m_captureType = CaptureType::Desktop;
    m_windowHandle = getDesktopWindowHandle();
    m_windowTitle = "Desktop";
    
    // 데스크톱 크기 가져오기
    m_captureSize = getDesktopSize();
    
    m_isOpen = true;
    m_lastCaptureTime = std::chrono::steady_clock::now();
    
    std::cout << "Opened desktop capture: " 
              << m_captureSize.width << "x" << m_captureSize.height << std::endl;
    
    return true;
}

bool WindowCaptureSource::openRegion(cv::Rect region) {
    if (region.width <= 0 || region.height <= 0) {
        m_lastError = "Invalid capture region";
        return false;
    }
    
    m_captureType = CaptureType::Region;
    m_captureRegion = region;
    m_captureSize = cv::Size(region.width, region.height);
    
    m_isOpen = true;
    m_lastCaptureTime = std::chrono::steady_clock::now();
    
    std::cout << "Opened region capture: " << region.x << "," << region.y 
              << " " << region.width << "x" << region.height << std::endl;
    
    return true;
}

void WindowCaptureSource::setCaptureType(CaptureType type) {
    m_captureType = type;
}

WindowCaptureSource::CaptureType WindowCaptureSource::getCaptureType() const {
    return m_captureType;
}

void WindowCaptureSource::setWindowHandle(void* handle) {
    m_windowHandle = handle;
}

void* WindowCaptureSource::getWindowHandle() const {
    return m_windowHandle;
}

void WindowCaptureSource::setWindowTitle(const std::string& title) {
    m_windowTitle = title;
}

std::string WindowCaptureSource::getWindowTitle() const {
    return m_windowTitle;
}

void WindowCaptureSource::setCaptureRegion(cv::Rect region) {
    m_captureRegion = region;
    m_captureSize = cv::Size(region.width, region.height);
}

cv::Rect WindowCaptureSource::getCaptureRegion() const {
    return m_captureRegion;
}

void WindowCaptureSource::setCaptureSize(cv::Size size) {
    m_captureSize = size;
}

cv::Size WindowCaptureSource::getCaptureSize() const {
    return m_captureSize;
}

void WindowCaptureSource::setFPS(double fps) {
    m_fps = std::max(1.0, std::min(120.0, fps));
}

double WindowCaptureSource::getFPS() const {
    return m_fps;
}

void WindowCaptureSource::setIncludeBorders(bool include) {
    m_includeBorders = include;
}

bool WindowCaptureSource::getIncludeBorders() const {
    return m_includeBorders;
}

void WindowCaptureSource::setIncludeCursor(bool include) {
    m_includeCursor = include;
}

bool WindowCaptureSource::getIncludeCursor() const {
    return m_includeCursor;
}

// 윈도우 정보 메서드

std::vector<WindowCaptureSource::WindowInfo> WindowCaptureSource::getAvailableWindows() {
    std::vector<WindowInfo> windows;
    
#ifdef _WIN32
    // Windows API로 윈도우 목록 가져오기
    HWND hwnd = nullptr;
    while ((hwnd = FindWindowEx(nullptr, hwnd, nullptr, nullptr)) != nullptr) {
        if (IsWindowVisible(hwnd)) {
            WindowInfo info = collectWindowInfo(hwnd);
            if (!info.title.empty()) {
                windows.push_back(info);
            }
        }
    }
#endif
    
    return windows;
}

WindowCaptureSource::WindowInfo WindowCaptureSource::findWindowByTitle(const std::string& title) {
    void* handle = findWindowHandleByTitle(title);
    if (handle) {
        return collectWindowInfo(handle);
    }
    
    WindowInfo notFound;
    notFound.title = "Not Found";
    return notFound;
}

WindowCaptureSource::WindowInfo WindowCaptureSource::findWindowByClassName(const std::string& className) {
    void* handle = findWindowHandleByClassName(className);
    if (handle) {
        return collectWindowInfo(handle);
    }
    
    WindowInfo notFound;
    notFound.title = "Not Found";
    return notFound;
}

WindowCaptureSource::WindowInfo WindowCaptureSource::getActiveWindow() {
    void* handle = getActiveWindowHandle();
    if (handle) {
        return collectWindowInfo(handle);
    }
    
    WindowInfo notFound;
    notFound.title = "No Active Window";
    return notFound;
}

WindowCaptureSource::WindowInfo WindowCaptureSource::getDesktopWindow() {
    void* handle = getDesktopWindowHandle();
    if (handle) {
        return collectWindowInfo(handle);
    }
    
    WindowInfo desktop;
    desktop.title = "Desktop";
    desktop.handle = handle;
    desktop.rect = getDesktopRect();
    desktop.isVisible = true;
    return desktop;
}

// 캡처 메서드

cv::Mat WindowCaptureSource::getCurrentFrame() const {
    return m_currentFrameMat.clone();
}

bool WindowCaptureSource::captureFrame(cv::Mat& frame) {
    return readNextFrame(frame);
}

bool WindowCaptureSource::captureFrameWithTimeout(cv::Mat& frame, int timeoutMs) {
    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        if (readNextFrame(frame)) {
            return true;
        }
        
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();
        
        if (elapsed >= timeoutMs) {
            m_lastError = "Frame capture timeout";
            return false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// 화면 정보 메서드

cv::Size WindowCaptureSource::getDesktopSize() {
#ifdef _WIN32
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    return cv::Size(width, height);
#else
    return cv::Size(1920, 1080); // 기본값
#endif
}

cv::Rect WindowCaptureSource::getDesktopRect() {
    cv::Size size = getDesktopSize();
    return cv::Rect(0, 0, size.width, size.height);
}

int WindowCaptureSource::getDesktopDPI() {
#ifdef _WIN32
    HDC hdc = GetDC(nullptr);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(nullptr, hdc);
    return dpi;
#else
    return 96; // 기본 DPI
#endif
}

// 윈도우 제어 메서드

bool WindowCaptureSource::bringWindowToFront(void* windowHandle) {
#ifdef _WIN32
    return SetForegroundWindow(static_cast<HWND>(windowHandle)) != FALSE;
#else
    return false;
#endif
}

bool WindowCaptureSource::minimizeWindow(void* windowHandle) {
#ifdef _WIN32
    return ShowWindow(static_cast<HWND>(windowHandle), SW_MINIMIZE) != FALSE;
#else
    return false;
#endif
}

bool WindowCaptureSource::maximizeWindow(void* windowHandle) {
#ifdef _WIN32
    return ShowWindow(static_cast<HWND>(windowHandle), SW_MAXIMIZE) != FALSE;
#else
    return false;
#endif
}

bool WindowCaptureSource::restoreWindow(void* windowHandle) {
#ifdef _WIN32
    return ShowWindow(static_cast<HWND>(windowHandle), SW_RESTORE) != FALSE;
#else
    return false;
#endif
}

bool WindowCaptureSource::isWindowVisible(void* windowHandle) {
#ifdef _WIN32
    return IsWindowVisible(static_cast<HWND>(windowHandle)) != FALSE;
#else
    return false;
#endif
}

bool WindowCaptureSource::isWindowMinimized(void* windowHandle) {
#ifdef _WIN32
    return IsIconic(static_cast<HWND>(windowHandle)) != FALSE;
#else
    return false;
#endif
}

// 스트리밍 메서드

bool WindowCaptureSource::startStreaming() {
    return isOpen();
}

bool WindowCaptureSource::stopStreaming() {
    return true;
}

bool WindowCaptureSource::isStreaming() const {
    return isOpen();
}

double WindowCaptureSource::getCurrentFPS() const {
    // 실시간 FPS 계산 (간단한 구현)
    return m_fps;
}

// 영역 관련 메서드

void WindowCaptureSource::setCaptureRegionByCenter(cv::Point center, cv::Size size) {
    cv::Point topLeft(center.x - size.width / 2, center.y - size.height / 2);
    setCaptureRegion(cv::Rect(topLeft, size));
}

void WindowCaptureSource::setCaptureRegionByCorners(cv::Point topLeft, cv::Point bottomRight) {
    cv::Size size(bottomRight.x - topLeft.x, bottomRight.y - topLeft.y);
    setCaptureRegion(cv::Rect(topLeft, size));
}

bool WindowCaptureSource::isValidCaptureRegion() const {
    return m_captureRegion.width > 0 && m_captureRegion.height > 0;
}

// 고급 캡처 옵션

void WindowCaptureSource::setPixelFormat(int format) {
    // 픽셀 포맷 설정 (구현 필요)
}

int WindowCaptureSource::getPixelFormat() const {
    return CV_8UC4; // 기본값
}

void WindowCaptureSource::setCompressionLevel(int level) {
    // 압축 레벨 설정 (구현 필요)
}

int WindowCaptureSource::getCompressionLevel() const {
    return 0; // 기본값
}

// Private 메서드

void WindowCaptureSource::updateAvailableWindows() {
    m_availableWindows = getAvailableWindows();
}

bool WindowCaptureSource::captureWindowFrame(void* windowHandle, cv::Mat& frame) {
#ifdef _WIN32
    HWND hwnd = static_cast<HWND>(windowHandle);
    
    // 윈도우 DC 가져오기
    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    
    if (!hdcWindow || !hdcMemDC) {
        m_lastError = "Failed to get device context";
        return false;
    }
    
    // 비트맵 생성
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, m_captureSize.width, m_captureSize.height);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMemDC, hbmScreen);
    
    // 윈도우 내용 복사
    BOOL success = BitBlt(hdcMemDC, 0, 0, m_captureSize.width, m_captureSize.height,
                         hdcWindow, 0, 0, SRCCOPY);
    
    if (success) {
        // OpenCV Mat로 변환
        BITMAPINFOHEADER bi = {0};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = m_captureSize.width;
        bi.biHeight = -m_captureSize.height; // 상하 반전 방지
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        
        frame.create(m_captureSize.height, m_captureSize.width, CV_8UC4);
        GetDIBits(hdcMemDC, hbmScreen, 0, m_captureSize.height, 
                  frame.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        
        // 커서 추가 (옵션)
        if (m_includeCursor) {
            cv::Point cursorPos = getCurrentCursorPosition();
            drawCursor(frame, cursorPos);
        }
    }
    
    // 정리
    SelectObject(hdcMemDC, hbmOld);
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);
    
    return success != FALSE;
#else
    m_lastError = "Window capture not supported on this platform";
    return false;
#endif
}

bool WindowCaptureSource::captureDesktopFrame(cv::Mat& frame) {
#ifdef _WIN32
    // 전체 화면 캡처
    HDC hdcDesktop = GetDC(nullptr);
    HDC hdcMemDC = CreateCompatibleDC(hdcDesktop);
    
    if (!hdcDesktop || !hdcMemDC) {
        m_lastError = "Failed to get desktop device context";
        return false;
    }
    
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcDesktop, m_captureSize.width, m_captureSize.height);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMemDC, hbmScreen);
    
    BOOL success = BitBlt(hdcMemDC, 0, 0, m_captureSize.width, m_captureSize.height,
                         hdcDesktop, 0, 0, SRCCOPY);
    
    if (success) {
        BITMAPINFOHEADER bi = {0};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = m_captureSize.width;
        bi.biHeight = -m_captureSize.height;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        
        frame.create(m_captureSize.height, m_captureSize.width, CV_8UC4);
        GetDIBits(hdcMemDC, hbmScreen, 0, m_captureSize.height, 
                  frame.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        
        if (m_includeCursor) {
            cv::Point cursorPos = getCurrentCursorPosition();
            drawCursor(frame, cursorPos);
        }
    }
    
    SelectObject(hdcMemDC, hbmOld);
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(nullptr, hdcDesktop);
    
    return success != FALSE;
#else
    m_lastError = "Desktop capture not supported on this platform";
    return false;
#endif
}

bool WindowCaptureSource::captureRegionFrame(cv::Rect region, cv::Mat& frame) {
#ifdef _WIN32
    HDC hdcDesktop = GetDC(nullptr);
    HDC hdcMemDC = CreateCompatibleDC(hdcDesktop);
    
    if (!hdcDesktop || !hdcMemDC) {
        m_lastError = "Failed to get device context";
        return false;
    }
    
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcDesktop, region.width, region.height);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMemDC, hbmScreen);
    
    BOOL success = BitBlt(hdcMemDC, 0, 0, region.width, region.height,
                         hdcDesktop, region.x, region.y, SRCCOPY);
    
    if (success) {
        BITMAPINFOHEADER bi = {0};
        bi.biSize = sizeof(BITMAPINFOHEADER);
        bi.biWidth = region.width;
        bi.biHeight = -region.height;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        
        frame.create(region.height, region.width, CV_8UC4);
        GetDIBits(hdcMemDC, hbmScreen, 0, region.height, 
                  frame.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    }
    
    SelectObject(hdcMemDC, hbmOld);
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(nullptr, hdcDesktop);
    
    return success != FALSE;
#else
    m_lastError = "Region capture not supported on this platform";
    return false;
#endif
}

WindowCaptureSource::WindowInfo WindowCaptureSource::collectWindowInfo(void* windowHandle) {
    WindowInfo info;
    info.handle = windowHandle;
    info.title = getWindowTitle(windowHandle);
    info.className = getWindowClassName(windowHandle);
    info.rect = getWindowRect(windowHandle);
    info.isVisible = isWindowVisible(windowHandle);
    
    return info;
}

std::string WindowCaptureSource::getWindowTitle(void* windowHandle) {
#ifdef _WIN32
    HWND hwnd = static_cast<HWND>(windowHandle);
    int length = GetWindowTextLength(hwnd) + 1;
    std::vector<char> buffer(length);
    GetWindowText(hwnd, buffer.data(), length);
    return std::string(buffer.data());
#else
    return "";
#endif
}

std::string WindowCaptureSource::getWindowClassName(void* windowHandle) {
#ifdef _WIN32
    HWND hwnd = static_cast<HWND>(windowHandle);
    char buffer[256];
    GetClassName(hwnd, buffer, sizeof(buffer));
    return std::string(buffer);
#else
    return "";
#endif
}

cv::Rect WindowCaptureSource::getWindowRect(void* windowHandle) {
#ifdef _WIN32
    HWND hwnd = static_cast<HWND>(windowHandle);
    RECT rect;
    GetWindowRect(hwnd, &rect);
    return cv::Rect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
#else
    return cv::Rect(0, 0, 0, 0);
#endif
}

void* WindowCaptureSource::findWindowHandleByTitle(const std::string& title) {
#ifdef _WIN32
    HWND hwnd = FindWindow(nullptr, title.c_str());
    return hwnd;
#else
    return nullptr;
#endif
}

void* WindowCaptureSource::findWindowHandleByClassName(const std::string& className) {
#ifdef _WIN32
    HWND hwnd = FindWindow(className.c_str(), nullptr);
    return hwnd;
#else
    return nullptr;
#endif
}

void* WindowCaptureSource::getActiveWindowHandle() {
#ifdef _WIN32
    return GetForegroundWindow();
#else
    return nullptr;
#endif
}

void* WindowCaptureSource::getDesktopWindowHandle() {
#ifdef _WIN32
    return GetDesktopWindow();
#else
    return nullptr;
#endif
}

double WindowCaptureSource::calculateCaptureDelay() const {
    if (m_fps <= 0.0) {
        return 33.33; // 30 FPS 기본값
    }
    
    return 1000.0 / m_fps; // ms 단위
}

bool WindowCaptureSource::shouldCaptureNextFrame() const {
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastCaptureTime);
    
    double frameDelay = calculateCaptureDelay();
    return elapsed.count() >= frameDelay;
}

void WindowCaptureSource::convertPixelFormat(const cv::Mat& input, cv::Mat& output, int format) {
    // 픽셀 포맷 변환 (구현 필요)
    output = input.clone();
}

void WindowCaptureSource::drawCursor(cv::Mat& frame, cv::Point position) {
    // 간단한 커서 그리기 (구현 필요)
    cv::circle(frame, position, 5, cv::Scalar(255, 255, 255, 255), 2);
}

cv::Point WindowCaptureSource::getCurrentCursorPosition() {
#ifdef _WIN32
    POINT point;
    GetCursorPos(&point);
    return cv::Point(point.x, point.y);
#else
    return cv::Point(0, 0);
#endif
}
