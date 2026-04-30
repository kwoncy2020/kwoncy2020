#pragma once

#include <string>
#include <chrono>
#include <opencv2/opencv.hpp>

/**
 * @brief 상태 바 UI 클래스
 * 
 * 이 클래스는 애플리케이션의 상태 바를 관리합니다.
 * 이미지 정보, 픽셀 좌표, 처리 상태 등을 표시합니다.
 * 
 * 주요 기능:
 * - 이미지 정보 표시 (크기, 채널 등)
 * - 픽셀 좌표 및 값 정보
 * - 처리 상태 및 진행률
 * - 성능 정보 (FPS, 처리 시간 등)
 */
class StatusBar {
private:
    // 이미지 정보
    std::string m_imagePath;                            // 이미지 파일 경로
    int m_imageWidth;                                   // 이미지 너비
    int m_imageHeight;                                  // 이미지 높이
    int m_imageChannels;                                // 이미지 채널 수
    size_t m_imageSize;                                 // 이미지 크기 (bytes)
    
    // 픽셀 정보
    int m_mouseX;                                       // 마우스 X 좌표
    int m_mouseY;                                       // 마우스 Y 좌표
    cv::Vec3b m_pixelValue;                             // 픽셀 값 (BGR)
    bool m_showPixelInfo;                               // 픽셀 정보 표시 여부
    bool m_isMouseOverImage;                            // 마우스가 이미지 위에 있는지
    
    // 처리 정보
    std::string m_processingStatus;                     // 처리 상태 메시지
    float m_processingProgress;                         // 처리 진행률 (0.0-1.0)
    bool m_isProcessing;                                // 처리 중 여부
    double m_processingTime;                            // 마지막 처리 시간 (ms)
    
    // 성능 정보
    double m_currentFPS;                                // 현재 FPS
    double m_averageFPS;                                // 평균 FPS
    std::chrono::steady_clock::time_point m_lastFPSUpdate; // 마지막 FPS 업데이트 시간
    int m_frameCount;                                   // 프레임 카운트
    std::chrono::steady_clock::time_point m_fpsStartTime;   // FPS 측정 시작 시간
    
    // 데이터 소스 정보
    std::string m_dataSourceType;                       // 데이터 소스 타입
    std::string m_dataSourceInfo;                       // 데이터 소스 정보
    bool m_isLiveSource;                                // 라이브 소스 여부
    
    // 메모리 정보
    size_t m_memoryUsage;                               // 메모리 사용량
    size_t m_totalMemory;                               // 전체 메모리
    
    // 상태 바 설정
    bool m_showImageInfo;                               // 이미지 정보 표시 여부
    bool m showPerformanceInfo;                         // 성능 정보 표시 여부
    bool m showMemoryInfo;                              // 메모리 정보 표시 여부
    bool m_showProcessingInfo;                         // 처리 정보 표시 여부

public:
    /**
     * @brief 생성자
     */
    StatusBar();
    
    /**
     * @brief 소멸자
     */
    ~StatusBar() = default;

    // 상태 바 렌더링
    void render();

    // 이미지 정보 설정
    void setImagePath(const std::string& path);
    void setImageInfo(int width, int height, int channels, size_t size);
    void clearImageInfo();
    
    // 픽셀 정보 설정
    void setMousePosition(int x, int y);
    void setPixelValue(const cv::Vec3b& value);
    void setMouseOverImage(bool over);
    void setShowPixelInfo(bool show);
    
    // 처리 정보 설정
    void setProcessingStatus(const std::string& status);
    void setProcessingProgress(float progress);
    void setProcessingState(bool isProcessing);
    void setProcessingTime(double timeMs);
    
    // 성능 정보 설정
    void updateFPS();
    void setCurrentFPS(double fps);
    void setAverageFPS(double fps);
    
    // 데이터 소스 정보 설정
    void setDataSourceType(const std::string& type);
    void setDataSourceInfo(const std::string& info);
    void setIsLiveSource(bool isLive);
    
    // 메모리 정보 설정
    void setMemoryUsage(size_t usage, size_t total);
    
    // 표시 옵션 설정
    void setShowImageInfo(bool show);
    void setShowPerformanceInfo(bool show);
    void setShowMemoryInfo(bool show);
    void setShowProcessingInfo(bool show);

    // 상태 조회 메서드들
    std::string getImagePath() const;
    std::pair<int, int> getImageSize() const;
    int getImageChannels() const;
    std::pair<int, int> getMousePosition() const;
    cv::Vec3b getPixelValue() const;
    bool isMouseOverImage() const;
    std::string getProcessingStatus() const;
    float getProcessingProgress() const;
    bool isProcessing() const;
    double getCurrentFPS() const;
    double getAverageFPS() const;
    std::string getDataSourceType() const;
    bool isLiveSource() const;

private:
    /**
     * @brief 이미지 정보 섹션 렌더링
     */
    void renderImageInfo();
    
    /**
     * @brief 픽셀 정보 섹션 렌더링
     */
    void renderPixelInfo();
    
    /**
     * @brief 처리 정보 섹션 렌더링
     */
    void renderProcessingInfo();
    
    /**
     * @brief 성능 정보 섹션 렌더링
     */
    void renderPerformanceInfo();
    
    /**
     * @brief 데이터 소스 정보 섹션 렌더링
     */
    void renderDataSourceInfo();
    
    /**
     * @brief 메모리 정보 섹션 렌더링
     */
    void renderMemoryInfo();
    
    /**
     * @brief 구분선 렌더링
     */
    void renderSeparator();
    
    /**
     * @brief 파일 크기 포맷팅
     * @param bytes 바이트 수
     * @return 포맷된 크기 문자열
     */
    std::string formatFileSize(size_t bytes) const;
    
    /**
     * @brief 시간 포맷팅
     * @param timeMs 시간 (밀리초)
     * @return 포맷된 시간 문자열
     */
    std::string formatTime(double timeMs) const;
    
    /**
     * @brief FPS 포맷팅
     * @param fps FPS 값
     * @return 포맷된 FPS 문자열
     */
    std::string formatFPS(double fps) const;
    
    /**
     * @brief BGR 값을 16진수 문자열로 변환
     * @param value BGR 값
     * @return 16진수 문자열
     */
    std::string pixelValueToHex(const cv::Vec3b& value) const;
    
    /**
     * @brief 메모리 사용량 계산
     * @return 메모리 사용량 (bytes)
     */
    size_t calculateMemoryUsage() const;
    
    /**
     * @brief FPS 초기화
     */
    void initializeFPS();
    
    /**
     * @brief 상태 메시지 포맷팅
     * @param message 메시지
     * @param maxLength 최대 길이
     * @return 포맷된 메시지
     */
    std::string formatStatusMessage(const std::string& message, int maxLength = 50) const;
};
