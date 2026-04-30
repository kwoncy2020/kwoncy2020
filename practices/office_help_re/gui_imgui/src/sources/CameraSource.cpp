#include "sources/CameraSource.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>

CameraSource::CameraSource() 
    : m_cameraId(-1), m_resolution(640, 480), m_fps(30.0),
      m_isAutoReconnect(true), m_reconnectAttempts(3) {
    updateAvailableCameras();
}

CameraSource::~CameraSource() {
    close();
}

bool CameraSource::open(const std::string& source) {
    try {
        int cameraId = std::stoi(source);
        return openCamera(cameraId);
    } catch (const std::exception& e) {
        m_lastError = "Invalid camera ID: " + source;
        return false;
    }
}

bool CameraSource::close() {
    if (m_cameraCapture.isOpened()) {
        m_cameraCapture.release();
    }
    
    m_currentFrameMat.release();
    m_isOpen = false;
    
    std::cout << "Closed camera: " << m_cameraId << std::endl;
    return true;
}

bool CameraSource::isOpen() const {
    return m_isOpen && m_cameraCapture.isOpened();
}

bool CameraSource::readNextFrame(cv::Mat& frame) {
    if (!isOpen()) {
        if (m_isAutoReconnect) {
            return attemptReconnect() && m_cameraCapture.read(frame);
        } else {
            m_lastError = "Camera is not open";
            return false;
        }
    }
    
    if (!m_cameraCapture.read(frame)) {
        if (m_isAutoReconnect) {
            return attemptReconnect() && m_cameraCapture.read(frame);
        } else {
            m_lastError = "Failed to capture frame from camera";
            return false;
        }
    }
    
    m_currentFrameMat = frame.clone();
    m_lastFrameTime = std::chrono::steady_clock::now();
    return true;
}

bool CameraSource::readFrameAt(int frameIndex, cv::Mat& frame) {
    // 카메라는 라이브 소스이므로 특정 프레임 탐색 불가
    m_lastError = "Camera source does not support frame seeking";
    return false;
}

bool CameraSource::seekToFrame(int frameIndex) {
    // 카메라는 라이브 소스이므로 프레임 탐색 불가
    m_lastError = "Camera source does not support frame seeking";
    return false;
}

bool CameraSource::seekToTime(double timeInSeconds) {
    // 카메라는 라이브 소스이므로 시간 탐색 불가
    m_lastError = "Camera source does not support time seeking";
    return false;
}

DataSourceInfo CameraSource::getSourceInfo() const {
    DataSourceInfo info;
    info.sourceType = "Camera";
    info.sourcePath = "Camera " + std::to_string(m_cameraId);
    info.isLive = true;
    info.width = m_resolution.width;
    info.height = m_resolution.height;
    info.channels = 3; // 대부분의 카메라는 컬러
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

std::string CameraSource::getLastError() const {
    return m_lastError;
}

bool CameraSource::isLive() const {
    return true;
}

int CameraSource::getCurrentFrameIndex() const {
    return -1; // 카메라는 프레임 인덱스 개념 없음
}

double CameraSource::getCurrentTime() const {
    return -1.0; // 카메라는 시간 개념 없음
}

bool CameraSource::hasMoreFrames() const {
    return isOpen(); // 카메라는 항상 프레임이 있음
}

void CameraSource::reset() {
    // 카메라는 리셋 의미가 없음, 재연결 수행
    if (m_isAutoReconnect) {
        reconnect();
    }
}

// 카메라 특화 메서드

bool CameraSource::openCamera(int cameraId) {
    return openCamera(cameraId, m_resolution, m_fps);
}

bool CameraSource::openCamera(int cameraId, cv::Size resolution, double fps) {
    m_cameraId = cameraId;
    m_lastError.clear();
    
    // 카메라 열기
    if (!m_cameraCapture.open(cameraId)) {
        m_lastError = "Failed to open camera: " + std::to_string(cameraId);
        m_isOpen = false;
        return false;
    }
    
    // 해상도 설정
    if (!setResolution(resolution)) {
        std::cerr << "Warning: Failed to set resolution, using default" << std::endl;
    }
    
    // FPS 설정 (지원되는 경우)
    if (!setFPS(fps)) {
        std::cerr << "Warning: Failed to set FPS, using default" << std::endl;
    }
    
    // 카메라 속성 업데이트
    updateCameraProperties();
    
    m_isOpen = true;
    m_lastFrameTime = std::chrono::steady_clock::now();
    
    std::cout << "Opened camera: " << cameraId 
              << " (" << m_resolution.width << "x" << m_resolution.height 
              << ", " << m_fps << " FPS)" << std::endl;
    
    return true;
}

int CameraSource::getCameraId() const {
    return m_cameraId;
}

void CameraSource::setCameraId(int cameraId) {
    if (isOpen()) {
        close();
    }
    m_cameraId = cameraId;
}

void CameraSource::setAutoReconnect(bool enable) {
    m_isAutoReconnect = enable;
}

bool CameraSource::getAutoReconnect() const {
    return m_isAutoReconnect;
}

void CameraSource::setReconnectAttempts(int attempts) {
    m_reconnectAttempts = std::max(1, attempts);
}

int CameraSource::getReconnectAttempts() const {
    return m_reconnectAttempts;
}

// 카메라 속성 제어

bool CameraSource::setResolution(cv::Size resolution) {
    if (!isOpen()) {
        return false;
    }
    
    bool success = m_cameraCapture.set(cv::CAP_PROP_FRAME_WIDTH, resolution.width) &&
                   m_cameraCapture.set(cv::CAP_PROP_FRAME_HEIGHT, resolution.height);
    
    if (success) {
        m_resolution = resolution;
    }
    
    return success;
}

cv::Size CameraSource::getResolution() const {
    return m_resolution;
}

bool CameraSource::setFPS(double fps) {
    if (!isOpen()) {
        return false;
    }
    
    bool success = m_cameraCapture.set(cv::CAP_PROP_FPS, fps);
    
    if (success) {
        m_fps = fps;
    }
    
    return success;
}

double CameraSource::getFPS() const {
    return m_fps;
}

bool CameraSource::setBrightness(double brightness) {
    return setCameraProperty(cv::CAP_PROP_BRIGHTNESS, brightness);
}

double CameraSource::getBrightness() const {
    return getCameraProperty(cv::CAP_PROP_BRIGHTNESS);
}

bool CameraSource::setContrast(double contrast) {
    return setCameraProperty(cv::CAP_PROP_CONTRAST, contrast);
}

double CameraSource::getContrast() const {
    return getCameraProperty(cv::CAP_PROP_CONTRAST);
}

bool CameraSource::setSaturation(double saturation) {
    return setCameraProperty(cv::CAP_PROP_SATURATION, saturation);
}

double CameraSource::getSaturation() const {
    return getCameraProperty(cv::CAP_PROP_SATURATION);
}

bool CameraSource::setExposure(double exposure) {
    return setCameraProperty(cv::CAP_PROP_EXPOSURE, exposure);
}

double CameraSource::getExposure() const {
    return getCameraProperty(cv::CAP_PROP_EXPOSURE);
}

bool CameraSource::setGain(double gain) {
    return setCameraProperty(cv::CAP_PROP_GAIN, gain);
}

double CameraSource::getGain() const {
    return getCameraProperty(cv::CAP_PROP_GAIN);
}

bool CameraSource::setWhiteBalance(double whiteBalance) {
    return setCameraProperty(cv::CAP_PROP_WHITE_BALANCE, whiteBalance);
}

double CameraSource::getWhiteBalance() const {
    return getCameraProperty(cv::CAP_PROP_WHITE_BALANCE);
}

bool CameraSource::setFocus(int focus) {
    return setCameraProperty(cv::CAP_PROP_FOCUS, static_cast<double>(focus));
}

int CameraSource::getFocus() const {
    return static_cast<int>(getCameraProperty(cv::CAP_PROP_FOCUS));
}

bool CameraSource::setZoom(double zoom) {
    return setCameraProperty(cv::CAP_PROP_ZOOM, zoom);
}

double CameraSource::getZoom() const {
    return getCameraProperty(cv::CAP_PROP_ZOOM);
}

// 카메라 정보 메서드

std::vector<CameraSource::CameraInfo> CameraSource::getAvailableCameras() {
    std::vector<CameraInfo> cameras;
    
    // 일반적으로 0-9번 카메라 확인
    for (int i = 0; i < 10; ++i) {
        CameraInfo info = collectCameraInfo(i);
        if (info.isAvailable) {
            cameras.push_back(info);
        }
    }
    
    return cameras;
}

CameraSource::CameraInfo CameraSource::getCameraInfo(int cameraId) {
    return collectCameraInfo(cameraId);
}

bool CameraSource::isCameraAvailable(int cameraId) {
    cv::VideoCapture testCapture;
    bool available = testCapture.open(cameraId);
    if (available) {
        testCapture.release();
    }
    return available;
}

// 프레임 처리 메서드

cv::Mat CameraSource::getCurrentFrame() const {
    return m_currentFrameMat.clone();
}

bool CameraSource::captureFrame(cv::Mat& frame) {
    return readNextFrame(frame);
}

bool CameraSource::captureFrameWithTimeout(cv::Mat& frame, int timeoutMs) {
    if (!isOpen()) {
        return false;
    }
    
    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        if (m_cameraCapture.read(frame)) {
            m_currentFrameMat = frame.clone();
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

// 스트리밍 메서드

bool CameraSource::startStreaming() {
    return isOpen();
}

bool CameraSource::stopStreaming() {
    return true; // 카메라는 항상 스트리밍 가능
}

bool CameraSource::isStreaming() const {
    return isOpen();
}

double CameraSource::getCurrentFPS() const {
    return calculateCurrentFPS();
}

int CameraSource::getTotalFramesCaptured() const {
    // OpenCV는 캡처된 프레임 수를 직접 제공하지 않음
    // 필요한 경우 내부 카운터 구현 가능
    return -1;
}

// 카메라 제어 메서드

bool CameraSource::autoFocus() {
    // OpenCV는 자동 초점 API를 직접 제공하지 않음
    // 특정 카메라 드라이버에 따라 다름
    return false;
}

bool CameraSource::setAutoFocus(bool enable) {
    // OpenCV는 자동 초점 API를 직접 제공하지 않음
    return false;
}

bool CameraSource::getAutoFocus() const {
    return false;
}

bool CameraSource::setAutoExposure(bool enable) {
    // 일부 카메라에서는 지원
    return setCameraProperty(cv::CAP_PROP_AUTO_EXPOSURE, enable ? 1.0 : 0.0);
}

bool CameraSource::getAutoExposure() const {
    double value = getCameraProperty(cv::CAP_PROP_AUTO_EXPOSURE);
    return value > 0.5;
}

bool CameraSource::setAutoWhiteBalance(bool enable) {
    // 일부 카메라에서는 지원
    return setCameraProperty(cv::CAP_PROP_AUTO_WB, enable ? 1.0 : 0.0);
}

bool CameraSource::getAutoWhiteBalance() const {
    double value = getCameraProperty(cv::CAP_PROP_AUTO_WB);
    return value > 0.5;
}

// 장치 관리 메서드

bool CameraSource::reconnect() {
    if (isOpen()) {
        close();
    }
    
    return openCamera(m_cameraId, m_resolution, m_fps);
}

bool CameraSource::testConnection() {
    if (!isOpen()) {
        return false;
    }
    
    cv::Mat testFrame;
    return m_cameraCapture.read(testFrame) && !testFrame.empty();
}

std::string CameraSource::getCameraBackend() const {
    return detectCameraBackend();
}

// Private 메서드

void CameraSource::updateCameraProperties() {
    if (!isOpen()) {
        return;
    }
    
    // 실제 카메라 속성 읽기
    int width = static_cast<int>(m_cameraCapture.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(m_cameraCapture.get(cv::CAP_PROP_FRAME_HEIGHT));
    m_resolution = cv::Size(width, height);
    
    double fps = m_cameraCapture.get(cv::CAP_PROP_FPS);
    if (fps > 0.0) {
        m_fps = fps;
    }
}

void CameraSource::updateAvailableCameras() {
    m_availableCameras = getAvailableCameras();
}

bool CameraSource::attemptReconnect() {
    for (int i = 0; i < m_reconnectAttempts; ++i) {
        std::cout << "Attempting to reconnect camera " << m_cameraId 
                  << " (attempt " << (i + 1) << "/" << m_reconnectAttempts << ")" << std::endl;
        
        if (reconnect()) {
            std::cout << "Camera reconnected successfully" << std::endl;
            return true;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    std::cerr << "Failed to reconnect camera after " << m_reconnectAttempts << " attempts" << std::endl;
    return false;
}

bool CameraSource::setCameraProperty(int property, double value) {
    if (!isOpen()) {
        return false;
    }
    
    return m_cameraCapture.set(property, value);
}

double CameraSource::getCameraProperty(int property) const {
    if (!isOpen()) {
        return -1.0;
    }
    
    return m_cameraCapture.get(property);
}

CameraSource::CameraInfo CameraSource::collectCameraInfo(int cameraId) {
    CameraInfo info;
    info.deviceId = cameraId;
    info.deviceName = "Camera " + std::to_string(cameraId);
    info.driverName = "Unknown";
    info.isAvailable = false;
    
    cv::VideoCapture testCapture;
    if (testCapture.open(cameraId)) {
        info.isAvailable = true;
        
        // 해상도 정보
        int width = static_cast<int>(testCapture.get(cv::CAP_PROP_FRAME_WIDTH));
        int height = static_cast<int>(testCapture.get(cv::CAP_PROP_FRAME_HEIGHT));
        info.resolution = cv::Size(width, height);
        
        // FPS 정보
        double fps = testCapture.get(cv::CAP_PROP_FPS);
        if (fps > 0.0) {
            info.supportedFPS.push_back(fps);
        } else {
            // 기본 FPS 추가
            info.supportedFPS = {15.0, 30.0, 60.0};
        }
        
        testCapture.release();
    }
    
    return info;
}

std::vector<cv::Size> CameraSource::getSupportedResolutions(int cameraId) {
    std::vector<cv::Size> resolutions;
    
    // 일반적인 해상도 목록
    std::vector<cv::Size> commonResolutions = {
        {320, 240}, {640, 480}, {800, 600}, {1024, 768},
        {1280, 720}, {1920, 1080}, {2560, 1440}, {3840, 2160}
    };
    
    cv::VideoCapture testCapture;
    if (testCapture.open(cameraId)) {
        for (const auto& res : commonResolutions) {
            testCapture.set(cv::CAP_PROP_FRAME_WIDTH, res.width);
            testCapture.set(cv::CAP_PROP_FRAME_HEIGHT, res.height);
            
            int actualWidth = static_cast<int>(testCapture.get(cv::CAP_PROP_FRAME_WIDTH));
            int actualHeight = static_cast<int>(testCapture.get(cv::CAP_PROP_FRAME_HEIGHT));
            
            if (actualWidth == res.width && actualHeight == res.height) {
                resolutions.push_back(res);
            }
        }
        testCapture.release();
    }
    
    return resolutions;
}

std::vector<double> CameraSource::getSupportedFPS(int cameraId) {
    std::vector<double> fpsValues;
    
    cv::VideoCapture testCapture;
    if (testCapture.open(cameraId)) {
        std::vector<double> commonFPS = {15.0, 30.0, 60.0, 120.0};
        
        for (double fps : commonFPS) {
            testCapture.set(cv::CAP_PROP_FPS, fps);
            double actualFPS = testCapture.get(cv::CAP_PROP_FPS);
            
            if (std::abs(actualFPS - fps) < 1.0) {
                fpsValues.push_back(fps);
            }
        }
        testCapture.release();
    }
    
    return fpsValues;
}

std::string CameraSource::detectCameraBackend() const {
    // OpenCV 빌드 정보에서 백엔드 감지
    std::ostringstream oss;
    
#ifdef CV_VERSION_MAJOR
    oss << "OpenCV " << CV_VERSION_MAJOR << "." << CV_VERSION_MINOR << "." << CV_VERSION_REVISION;
#endif
    
    // 플랫폼별 백엔드 정보
#ifdef _WIN32
    oss << " (DirectShow/V4L2)";
#elif __linux__
    oss << " (V4L2)";
#elif __APPLE__
    oss << " (AVFoundation)";
#else
    oss << " (Unknown)";
#endif
    
    return oss.str();
}

double CameraSource::calculateCurrentFPS() const {
    // 실시간 FPS 계산을 위한 타이밍 정보 필요
    // 간단한 구현으로 설정된 FPS 반환
    return m_fps;
}

bool CameraSource::isCameraConnected() const {
    return isOpen() && isCameraAvailable(m_cameraId);
}
