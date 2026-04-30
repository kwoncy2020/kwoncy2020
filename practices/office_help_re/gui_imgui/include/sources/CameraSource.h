#pragma once

#include "core/IDataSource.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

/**
 * @brief 카메라 데이터 소스
 * 
 * 이 클래스는 카메라 장치에서 실시간 비디오 스트림을
 * 읽어오는 데이터 소스를 구현합니다.
 * 
 * 주요 기능:
 * - 다양한 카메라 장치 지원
 * - 실시간 비디오 스트리밍
 * - 카메라 속성 제어 (해상도, FPS, 노출 등)
 * - 장치 목록 및 정보 제공
 */
class CameraSource : public IDataSource {
public:
    /**
     * @brief 카메라 정보 구조체
     */
    struct CameraInfo {
        int deviceId;              // 장치 ID
        std::string deviceName;   // 장치 이름
        std::string driverName;   // 드라이버 이름
        cv::Size resolution;      // 지원되는 해상도
        std::vector<double> supportedFPS; // 지원되는 FPS 목록
        bool isAvailable;         // 사용 가능 여부
        
        CameraInfo() : deviceId(-1), isAvailable(false) {}
    };

private:
    cv::VideoCapture m_cameraCapture;                 // OpenCV 카메라 캡처 객체
    int m_cameraId;                                   // 카메라 장치 ID
    cv::Size m_resolution;                            // 카메라 해상도
    double m_fps;                                     // 카메라 FPS
    bool m_isAutoReconnect;                           // 자동 재연결 여부
    int m_reconnectAttempts;                          // 재연결 시도 횟수
    std::chrono::steady_clock::time_point m_lastFrameTime; // 마지막 프레임 시간
    cv::Mat m_currentFrameMat;                        // 현재 프레임 이미지
    std::vector<CameraInfo> m_availableCameras;       // 사용 가능한 카메라 목록

public:
    /**
     * @brief 생성자
     */
    CameraSource();
    
    /**
     * @brief 소멸자
     */
    ~CameraSource() override;

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

    // 카메라 특화 메서드
    bool openCamera(int cameraId);
    bool openCamera(int cameraId, cv::Size resolution, double fps);
    
    int getCameraId() const;
    void setCameraId(int cameraId);
    
    void setAutoReconnect(bool enable);
    bool getAutoReconnect() const;
    
    void setReconnectAttempts(int attempts);
    int getReconnectAttempts() const;
    
    // 카메라 속성 제어
    bool setResolution(cv::Size resolution);
    cv::Size getResolution() const;
    
    bool setFPS(double fps);
    double getFPS() const;
    
    bool setBrightness(double brightness);
    double getBrightness() const;
    
    bool setContrast(double contrast);
    double getContrast() const;
    
    bool setSaturation(double saturation);
    double getSaturation() const;
    
    bool setExposure(double exposure);
    double getExposure() const;
    
    bool setGain(double gain);
    double getGain() const;
    
    bool setWhiteBalance(double whiteBalance);
    double getWhiteBalance() const;
    
    bool setFocus(int focus);
    int getFocus() const;
    
    bool setZoom(double zoom);
    double getZoom() const;
    
    // 카메라 정보 메서드
    static std::vector<CameraInfo> getAvailableCameras();
    static CameraInfo getCameraInfo(int cameraId);
    static bool isCameraAvailable(int cameraId);
    
    // 프레임 처리 메서드
    cv::Mat getCurrentFrame() const;
    bool captureFrame(cv::Mat& frame);
    bool captureFrameWithTimeout(cv::Mat& frame, int timeoutMs);
    
    // 스트리밍 메서드
    bool startStreaming();
    bool stopStreaming();
    bool isStreaming() const;
    
    double getCurrentFPS() const;
    int getTotalFramesCaptured() const;
    
    // 카메라 제어 메서드
    bool autoFocus();
    bool setAutoFocus(bool enable);
    bool getAutoFocus() const;
    
    bool setAutoExposure(bool enable);
    bool getAutoExposure() const;
    
    bool setAutoWhiteBalance(bool enable);
    bool getAutoWhiteBalance() const;
    
    // 장치 관리 메서드
    bool reconnect();
    bool testConnection();
    std::string getCameraBackend() const;

private:
    /**
     * @brief 카메라 속성 업데이트
     */
    void updateCameraProperties();
    
    /**
     * @brief 사용 가능한 카메라 목록 업데이트
     */
    void updateAvailableCameras();
    
    /**
     * @brief 카메라 재연결 시도
     * @return 성공 여부
     */
    bool attemptReconnect();
    
    /**
     * @brief 카메라 속성 설정
     * @param property OpenCV 카메라 속성
     * @param value 설정값
     * @return 성공 여부
     */
    bool setCameraProperty(int property, double value);
    
    /**
     * @brief 카메라 속성 가져오기
     * @param property OpenCV 카메라 속성
     * @return 속성값
     */
    double getCameraProperty(int property) const;
    
    /**
     * @brief 카메라 정보 수집
     * @param cameraId 카메라 ID
     * @return 카메라 정보
     */
    static CameraInfo collectCameraInfo(int cameraId);
    
    /**
     * @brief 지원되는 해상도 목록 가져오기
     * @param cameraId 카메라 ID
     * @return 해상도 목록
     */
    static std::vector<cv::Size> getSupportedResolutions(int cameraId);
    
    /**
     * @brief 지원되는 FPS 목록 가져오기
     * @param cameraId 카메라 ID
     * @return FPS 목록
     */
    static std::vector<double> getSupportedFPS(int cameraId);
    
    /**
     * @brief 카메라 백엔드 감지
     * @return 백엔드 이름
     */
    std::string detectCameraBackend() const;
    
    /**
     * @brief 프레임 타이밍 계산
     * @return 현재 FPS
     */
    double calculateCurrentFPS() const;
    
    /**
     * @brief 카메라 연결 상태 확인
     * @return 연결 상태
     */
    bool isCameraConnected() const;
};
