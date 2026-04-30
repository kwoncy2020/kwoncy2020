#pragma once

#include "core/IDataSource.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>
#endif

/**
 * @brief 윈도우 캡처 데이터 소스
 * 
 * 이 클래스는 화면의 특정 윈도우나 전체 화면을
 * 캡처하는 데이터 소스를 구현합니다.
 * 
 * 주요 기능:
 * - 윈도우 핸들 기반 캡처
 * - 전체 화면 캡처
 * - 특정 영역 캡처
 * - 실시간 화면 스트리밍
 */
class WindowCaptureSource : public IDataSource {
public:
    /**
     * @brief 캡처 대상 타입 열거형
     */
    enum class CaptureType {
        Window,           // 특정 윈도우
        Desktop,          // 전체 데스크톱
        Region            // 특정 영역
    };

    /**
     * @brief 윈도우 정보 구조체
     */
    struct WindowInfo {
        std::string title;        // 윈도우 제목
        std::string className;    // 윈도우 클래스 이름
        void* handle;             // 윈도우 핸들
        cv::Rect rect;            // 윈도우 영역
        bool isVisible;           // 보이는지 여부
        
        WindowInfo() : handle(nullptr), isVisible(false) {}
    };

private:
    CaptureType m_captureType;                         // 캡처 타입
    void* m_windowHandle;                               // 윈도우 핸들
    std::string m_windowTitle;                          // 윈도우 제목
    cv::Rect m_captureRegion;                          // 캡처 영역
    cv::Size m_captureSize;                            // 캡처 크기
    double m_fps;                                       // 캡처 FPS
    bool m_includeBorders;                             // 테두리 포함 여부
    bool m_includeCursor;                              // 커서 포함 여부
    std::chrono::steady_clock::time_point m_lastCaptureTime; // 마지막 캡처 시간
    cv::Mat m_currentFrameMat;                         // 현재 프레임 이미지
    std::vector<WindowInfo> m_availableWindows;         // 사용 가능한 윈도우 목록

public:
    /**
     * @brief 생성자
     */
    WindowCaptureSource();
    
    /**
     * @brief 소멸자
     */
    ~WindowCaptureSource() override;

    // IDataSource 인터페이스 구현
    bool open(const std::string& source) override;
    bool close() override;
    bool isOpen() const override;
    bool readNextFrame(cv::Mat& frame) override;
    bool readFrameAt(int frameIndex, cv::Mat& frame) override;
    bool seekToFrame(int frameIndex) override;
    bool seekToTime(double timeInSeconds) override;
    DataSourceInfo getSourceInfo() const override;
    std::string getLastError() const override;
    bool isLive() const override;
    int getCurrentFrameIndex() const override;
    double getCurrentTime() const override;
    bool hasMoreFrames() const override;
    void reset() override;

    // 윈도우 캡처 특화 메서드
    bool openWindow(void* windowHandle);
    bool openWindow(const std::string& windowTitle);
    bool openDesktop();
    bool openRegion(cv::Rect region);
    
    void setCaptureType(CaptureType type);
    CaptureType getCaptureType() const;
    
    void setWindowHandle(void* handle);
    void* getWindowHandle() const;
    
    void setWindowTitle(const std::string& title);
    std::string getWindowTitle() const;
    
    void setCaptureRegion(cv::Rect region);
    cv::Rect getCaptureRegion() const;
    
    void setCaptureSize(cv::Size size);
    cv::Size getCaptureSize() const;
    
    void setFPS(double fps);
    double getFPS() const;
    
    void setIncludeBorders(bool include);
    bool getIncludeBorders() const;
    
    void setIncludeCursor(bool include);
    bool getIncludeCursor() const;
    
    // 윈도우 정보 메서드
    static std::vector<WindowInfo> getAvailableWindows();
    static WindowInfo findWindowByTitle(const std::string& title);
    static WindowInfo findWindowByClassName(const std::string& className);
    static WindowInfo getActiveWindow();
    static WindowInfo getDesktopWindow();
    
    // 캡처 메서드
    cv::Mat getCurrentFrame() const;
    bool captureFrame(cv::Mat& frame);
    bool captureFrameWithTimeout(cv::Mat& frame, int timeoutMs);
    
    // 화면 정보 메서드
    static cv::Size getDesktopSize();
    static cv::Rect getDesktopRect();
    static int getDesktopDPI();
    
    // 윈도우 제어 메서드
    static bool bringWindowToFront(void* windowHandle);
    static bool minimizeWindow(void* windowHandle);
    static bool maximizeWindow(void* windowHandle);
    static bool restoreWindow(void* windowHandle);
    static bool isWindowVisible(void* windowHandle);
    static bool isWindowMinimized(void* windowHandle);
    
    // 스트리밍 메서드
    bool startStreaming();
    bool stopStreaming();
    bool isStreaming() const;
    
    double getCurrentFPS() const;
    
    // 영역 관련 메서드
    void setCaptureRegionByCenter(cv::Point center, cv::Size size);
    void setCaptureRegionByCorners(cv::Point topLeft, cv::Point bottomRight);
    bool isValidCaptureRegion() const;
    
    // 고급 캡처 옵션
    void setPixelFormat(int format);
    int getPixelFormat() const;
    
    void setCompressionLevel(int level);
    int getCompressionLevel() const;

private:
    /**
     * @brief 윈도우 목록 업데이트
     */
    void updateAvailableWindows();
    
    /**
     * @brief 윈도우 핸들로 프레임 캡처
     * @param windowHandle 윈도우 핸들
     * @param frame 출력 프레임
     * @return 성공 여부
     */
    bool captureWindowFrame(void* windowHandle, cv::Mat& frame);
    
    /**
     * @brief 데스크톱 프레임 캡처
     * @param frame 출력 프레임
     * @return 성공 여부
     */
    bool captureDesktopFrame(cv::Mat& frame);
    
    /**
     * @brief 특정 영역 프레임 캡처
     * @param region 캡처 영역
     * @param frame 출력 프레임
     * @return 성공 여부
     */
    bool captureRegionFrame(cv::Rect region, cv::Mat& frame);
    
    /**
     * @brief 윈도우 정보 수집
     * @param windowHandle 윈도우 핸들
     * @return 윈도우 정보
     */
    static WindowInfo collectWindowInfo(void* windowHandle);
    
    /**
     * @brief 윈도우 제목 가져오기
     * @param windowHandle 윈도우 핸들
     * @return 윈도우 제목
     */
    static std::string getWindowTitle(void* windowHandle);
    
    /**
     * @brief 윈도우 클래스 이름 가져오기
     * @param windowHandle 윈도우 핸들
     * @return 클래스 이름
     */
    static std::string getWindowClassName(void* windowHandle);
    
    /**
     * @brief 윈도우 영역 가져오기
     * @param windowHandle 윈도우 핸들
     * @return 윈도우 영역
     */
    static cv::Rect getWindowRect(void* windowHandle);
    
    /**
     * @brief 윈도우 핸들 찾기 (제목으로)
     * @param title 윈도우 제목
     * @return 윈도우 핸들
     */
    static void* findWindowHandleByTitle(const std::string& title);
    
    /**
     * @brief 윈도우 핸들 찾기 (클래스 이름으로)
     * @param className 클래스 이름
     * @return 윈도우 핸들
     */
    static void* findWindowHandleByClassName(const std::string& className);
    
    /**
     * @brief 활성 윈도우 핸들 가져오기
     * @return 활성 윈도우 핸들
     */
    static void* getActiveWindowHandle();
    
    /**
     * @brief 데스크톱 윈도우 핸들 가져오기
     * @return 데스크톱 윈도우 핸들
     */
    static void* getDesktopWindowHandle();
    
    /**
     * @brief 캡처 타이밍 계산
     * @return 다음 캡처까지 대기 시간 (ms)
     */
    double calculateCaptureDelay() const;
    
    /**
     * @brief 다음 프레임 캡처 시간인지 확인
     * @return 캡처 시간인지 여부
     */
    bool shouldCaptureNextFrame() const;
    
    /**
     * @brief 픽셀 포맷 변환
     * @param input 입력 이미지
     * @param output 출력 이미지
     * @param format 목표 포맷
     */
    void convertPixelFormat(const cv::Mat& input, cv::Mat& output, int format);
    
    /**
     * @brief 커서 그리기
     * @param frame 대상 프레임
     * @param position 커서 위치
     */
    void drawCursor(cv::Mat& frame, cv::Point position);
    
    /**
     * @brief 현재 커서 위치 가져오기
     * @return 커서 위치
     */
    static cv::Point getCurrentCursorPosition();
};
