#include "ui/StatusBar.h"
#include "imgui.h"
#include <sstream>
#include <iomanip>

StatusBar::StatusBar() 
    : m_imageWidth(0), m_imageHeight(0), m_imageChannels(0), m_imageSize(0),
      m_mouseX(0), m_mouseY(0), m_pixelValue(0, 0, 0), m_showPixelInfo(true),
      m_isMouseOverImage(false), m_processingProgress(0.0f), m_isProcessing(false),
      m_processingTime(0.0), m_currentFPS(0.0), m_averageFPS(0.0),
      m_frameCount(0), m_isLiveSource(false), m_memoryUsage(0), m_totalMemory(0),
      m_showImageInfo(true), m_showPerformanceInfo(true), m_showMemoryInfo(false),
      m_showProcessingInfo(true) {
    initializeFPS();
}

void StatusBar::render() {
    // FPS 업데이트
    updateFPS();
    
    if (ImGui::BeginStatusBar()) {
        // 이미지 정보
        if (m_showImageInfo) {
            renderImageInfo();
            renderSeparator();
        }
        
        // 픽셀 정보
        if (m_showPixelInfo && m_isMouseOverImage) {
            renderPixelInfo();
            renderSeparator();
        }
        
        // 데이터 소스 정보
        renderDataSourceInfo();
        renderSeparator();
        
        // 처리 정보
        if (m_showProcessingInfo) {
            renderProcessingInfo();
            renderSeparator();
        }
        
        // 성능 정보
        if (m_showPerformanceInfo) {
            renderPerformanceInfo();
            renderSeparator();
        }
        
        // 메모리 정보
        if (m_showMemoryInfo) {
            renderMemoryInfo();
        }
        
        ImGui::EndStatusBar();
    }
}

// 이미지 정보 설정
void StatusBar::setImagePath(const std::string& path) {
    m_imagePath = path;
}

void StatusBar::setImageInfo(int width, int height, int channels, size_t size) {
    m_imageWidth = width;
    m_imageHeight = height;
    m_imageChannels = channels;
    m_imageSize = size;
}

void StatusBar::clearImageInfo() {
    m_imagePath.clear();
    m_imageWidth = 0;
    m_imageHeight = 0;
    m_imageChannels = 0;
    m_imageSize = 0;
}

// 픽셀 정보 설정
void StatusBar::setMousePosition(int x, int y) {
    m_mouseX = x;
    m_mouseY = y;
}

void StatusBar::setPixelValue(const cv::Vec3b& value) {
    m_pixelValue = value;
}

void StatusBar::setMouseOverImage(bool over) {
    m_isMouseOverImage = over;
}

void StatusBar::setShowPixelInfo(bool show) {
    m_showPixelInfo = show;
}

// 처리 정보 설정
void StatusBar::setProcessingStatus(const std::string& status) {
    m_processingStatus = status;
}

void StatusBar::setProcessingProgress(float progress) {
    m_processingProgress = std::max(0.0f, std::min(1.0f, progress));
}

void StatusBar::setProcessingState(bool isProcessing) {
    m_isProcessing = isProcessing;
}

void StatusBar::setProcessingTime(double timeMs) {
    m_processingTime = timeMs;
}

// 성능 정보 설정
void StatusBar::updateFPS() {
    auto currentTime = std::chrono::steady_clock::now();
    m_frameCount++;
    
    // 1초마다 FPS 업데이트
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_fpsStartTime);
    if (elapsed.count() >= 1000) {
        m_currentFPS = static_cast<double>(m_frameCount) * 1000.0 / elapsed.count();
        
        // 평균 FPS 업데이트 (단순 이동 평균)
        if (m_averageFPS == 0.0) {
            m_averageFPS = m_currentFPS;
        } else {
            m_averageFPS = 0.9 * m_averageFPS + 0.1 * m_currentFPS;
        }
        
        // 리셋
        m_frameCount = 0;
        m_fpsStartTime = currentTime;
    }
}

void StatusBar::setCurrentFPS(double fps) {
    m_currentFPS = fps;
}

void StatusBar::setAverageFPS(double fps) {
    m_averageFPS = fps;
}

// 데이터 소스 정보 설정
void StatusBar::setDataSourceType(const std::string& type) {
    m_dataSourceType = type;
}

void StatusBar::setDataSourceInfo(const std::string& info) {
    m_dataSourceInfo = info;
}

void StatusBar::setIsLiveSource(bool isLive) {
    m_isLiveSource = isLive;
}

// 메모리 정보 설정
void StatusBar::setMemoryUsage(size_t usage, size_t total) {
    m_memoryUsage = usage;
    m_totalMemory = total;
}

// 표시 옵션 설정
void StatusBar::setShowImageInfo(bool show) {
    m_showImageInfo = show;
}

void StatusBar::setShowPerformanceInfo(bool show) {
    m_showPerformanceInfo = show;
}

void StatusBar::setShowMemoryInfo(bool show) {
    m_showMemoryInfo = show;
}

void StatusBar::setShowProcessingInfo(bool show) {
    m_showProcessingInfo = show;
}

// 상태 조회 메서드들
std::string StatusBar::getImagePath() const {
    return m_imagePath;
}

std::pair<int, int> StatusBar::getImageSize() const {
    return {m_imageWidth, m_imageHeight};
}

int StatusBar::getImageChannels() const {
    return m_imageChannels;
}

std::pair<int, int> StatusBar::getMousePosition() const {
    return {m_mouseX, m_mouseY};
}

cv::Vec3b StatusBar::getPixelValue() const {
    return m_pixelValue;
}

bool StatusBar::isMouseOverImage() const {
    return m_isMouseOverImage;
}

std::string StatusBar::getProcessingStatus() const {
    return m_processingStatus;
}

float StatusBar::getProcessingProgress() const {
    return m_processingProgress;
}

bool StatusBar::isProcessing() const {
    return m_isProcessing;
}

double StatusBar::getCurrentFPS() const {
    return m_currentFPS;
}

double StatusBar::getAverageFPS() const {
    return m_averageFPS;
}

std::string StatusBar::getDataSourceType() const {
    return m_dataSourceType;
}

bool StatusBar::isLiveSource() const {
    return m_isLiveSource;
}

// Private 메서드

void StatusBar::renderImageInfo() {
    std::ostringstream info;
    
    if (!m_imagePath.empty()) {
        std::string fileName = m_imagePath.substr(m_imagePath.find_last_of("\\/") + 1);
        info << "File: " << formatStatusMessage(fileName, 30);
    }
    
    if (m_imageWidth > 0 && m_imageHeight > 0) {
        if (!info.str().empty()) info << " | ";
        info << m_imageWidth << "x" << m_imageHeight;
        
        if (m_imageChannels > 0) {
            info << " (" << m_imageChannels << "ch)";
        }
        
        if (m_imageSize > 0) {
            info << " | " << formatFileSize(m_imageSize);
        }
    }
    
    if (!info.str().empty()) {
        ImGui::Text("%s", info.str().c_str());
    }
}

void StatusBar::renderPixelInfo() {
    std::ostringstream info;
    
    info << "Pos: " << m_mouseX << "," << m_mouseY;
    info << " | RGB: " << static_cast<int>(m_pixelValue[2]) << "," 
                     << static_cast<int>(m_pixelValue[1]) << "," 
                     << static_cast<int>(m_pixelValue[0]);
    info << " | #" << pixelValueToHex(m_pixelValue);
    
    ImGui::Text("%s", info.str().c_str());
}

void StatusBar::renderProcessingInfo() {
    std::ostringstream info;
    
    if (m_isProcessing) {
        info << "Processing";
        
        if (m_processingProgress > 0.0f) {
            info << ": " << static_cast<int>(m_processingProgress * 100) << "%";
        }
        
        if (m_processingTime > 0.0) {
            info << " | Time: " << formatTime(m_processingTime);
        }
        
        // 진행률 바
        ImGui::SameLine();
        ImGui::ProgressBar(m_processingProgress, ImVec2(100, 0), nullptr);
    } else if (!m_processingStatus.empty()) {
        info << "Status: " << formatStatusMessage(m_processingStatus, 40);
        
        if (m_processingTime > 0.0) {
            info << " | Last: " << formatTime(m_processingTime);
        }
    }
    
    if (!info.str().empty()) {
        ImGui::Text("%s", info.str().c_str());
    }
}

void StatusBar::renderPerformanceInfo() {
    std::ostringstream info;
    
    info << "FPS: " << formatFPS(m_currentFPS);
    
    if (m_averageFPS > 0.0) {
        info << " | Avg: " << formatFPS(m_averageFPS);
    }
    
    if (m_isLiveSource) {
        info << " | LIVE";
    }
    
    ImGui::Text("%s", info.str().c_str());
}

void StatusBar::renderDataSourceInfo() {
    std::ostringstream info;
    
    if (!m_dataSourceType.empty()) {
        info << "Source: " << m_dataSourceType;
        
        if (!m_dataSourceInfo.empty()) {
            info << " | " << formatStatusMessage(m_dataSourceInfo, 30);
        }
    }
    
    if (!info.str().empty()) {
        ImGui::Text("%s", info.str().c_str());
    }
}

void StatusBar::renderMemoryInfo() {
    std::ostringstream info;
    
    if (m_memoryUsage > 0) {
        info << "Memory: " << formatFileSize(m_memoryUsage);
        
        if (m_totalMemory > 0) {
            double percentage = (static_cast<double>(m_memoryUsage) / m_totalMemory) * 100.0;
            info << " / " << formatFileSize(m_totalMemory) << " (" 
                 << std::fixed << std::setprecision(1) << percentage << "%)";
        }
    } else {
        // 자동 메모리 사용량 계산
        size_t usage = calculateMemoryUsage();
        if (usage > 0) {
            info << "Memory: " << formatFileSize(usage);
        }
    }
    
    if (!info.str().empty()) {
        ImGui::Text("%s", info.str().c_str());
    }
}

void StatusBar::renderSeparator() {
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
}

std::string StatusBar::formatFileSize(size_t bytes) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[unitIndex];
    return oss.str();
}

std::string StatusBar::formatTime(double timeMs) const {
    std::ostringstream oss;
    
    if (timeMs < 1000.0) {
        oss << std::fixed << std::setprecision(1) << timeMs << "ms";
    } else if (timeMs < 60000.0) {
        oss << std::fixed << std::setprecision(1) << (timeMs / 1000.0) << "s";
    } else {
        int minutes = static_cast<int>(timeMs / 60000.0);
        double seconds = std::fmod(timeMs / 1000.0, 60.0);
        oss << minutes << "m" << std::fixed << std::setprecision(1) << seconds << "s";
    }
    
    return oss.str();
}

std::string StatusBar::formatFPS(double fps) const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << fps;
    return oss.str();
}

std::string StatusBar::pixelValueToHex(const cv::Vec3b& value) const {
    std::ostringstream oss;
    oss << std::hex << std::setw(2) << std::setfill('0') 
        << static_cast<int>(value[2])
        << std::hex << std::setw(2) << std::setfill('0') 
        << static_cast<int>(value[1])
        << std::hex << std::setw(2) << std::setfill('0') 
        << static_cast<int>(value[0]);
    return oss.str();
}

size_t StatusBar::calculateMemoryUsage() const {
    // 간단한 메모리 사용량 계산
    // 실제 구현에서는 더 정확한 메모리 측정 필요
    size_t usage = 0;
    
    // 이미지 데이터 크기
    if (m_imageWidth > 0 && m_imageHeight > 0 && m_imageChannels > 0) {
        usage += static_cast<size_t>(m_imageWidth * m_imageHeight * m_imageChannels);
    }
    
    // 다른 데이터 구조 크기 추가 (추가 구현 필요)
    
    return usage;
}

void StatusBar::initializeFPS() {
    m_fpsStartTime = std::chrono::steady_clock::now();
    m_frameCount = 0;
    m_currentFPS = 0.0;
    m_averageFPS = 0.0;
}

std::string StatusBar::formatStatusMessage(const std::string& message, int maxLength) const {
    if (message.length() <= static_cast<size_t>(maxLength)) {
        return message;
    }
    
    return message.substr(0, maxLength - 3) + "...";
}
