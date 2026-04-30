#include "core/FrameBuffer.h"
#include <algorithm>
#include <iostream>

FrameBuffer::FrameBuffer(const std::string& name) 
    : m_name(name), m_isInitialized(false) {
}

FrameBuffer::~FrameBuffer() {
    cleanup();
}

int FrameBuffer::addLevel(int width, int height) {
    if (width <= 0 || height <= 0) {
        std::cerr << "Invalid buffer size: " << width << "x" << height << std::endl;
        return -1;
    }
    
    BufferLevel level(width, height);
    level.cpuData = cv::Mat::zeros(height, width, CV_8UC4); // 기본 RGBA 포맷
    level.gpuDirty = true;
    level.cpuDirty = false;
    
    m_levels.push_back(level);
    
    if (!m_isInitialized) {
        m_isInitialized = true;
    }
    
    return static_cast<int>(m_levels.size()) - 1;
}

void FrameBuffer::updateData(const cv::Mat& data) {
    if (data.empty()) {
        std::cerr << "Cannot update buffer with empty data" << std::endl;
        return;
    }
    
    if (m_levels.empty()) {
        addLevel(data.cols, data.rows);
    }
    
    // 레벨 0 (원본) 업데이트
    updateLevelData(0, data);
    
    // 다른 레벨들도 자동으로 리사이즈
    for (size_t i = 1; i < m_levels.size(); ++i) {
        BufferLevel& level = m_levels[i];
        cv::resize(data, level.cpuData, cv::Size(level.width, level.height));
        level.gpuDirty = true;
        level.cpuDirty = false;
    }
}

void FrameBuffer::updateLevelData(int level, const cv::Mat& data) {
    if (!isValidLevel(level)) {
        std::cerr << "Invalid level: " << level << std::endl;
        return;
    }
    
    BufferLevel& bufferLevel = m_levels[level];
    
    // 크기가 다르면 리사이즈
    if (data.cols != bufferLevel.width || data.rows != bufferLevel.height) {
        cv::resize(data, bufferLevel.cpuData, cv::Size(bufferLevel.width, bufferLevel.height));
    } else {
        // 포맷 변환 필요 시
        if (data.type() != CV_8UC4) {
            cv::cvtColor(data, bufferLevel.cpuData, cv::COLOR_BGR2RGBA);
        } else {
            data.copyTo(bufferLevel.cpuData);
        }
    }
    
    bufferLevel.gpuDirty = true;
    bufferLevel.cpuDirty = false;
}

void FrameBuffer::updateTexture(int level, void* textureId) {
    if (!isValidLevel(level)) {
        std::cerr << "Invalid level: " << level << std::endl;
        return;
    }
    
    m_levels[level].gpuTexture = textureId;
    m_levels[level].gpuDirty = false;
}

void* FrameBuffer::getGPUTexture(int level) const {
    if (!isValidLevel(level)) {
        return nullptr;
    }
    
    return m_levels[level].gpuTexture;
}

void* FrameBuffer::getPreviewTexture(int maxWidth, int maxHeight) const {
    int optimalLevel = findOptimalPreviewLevel(maxWidth, maxHeight);
    if (optimalLevel >= 0) {
        return getGPUTexture(optimalLevel);
    }
    
    return nullptr;
}

cv::Mat FrameBuffer::getCPUData(int level) const {
    if (!isValidLevel(level)) {
        return cv::Mat();
    }
    
    return m_levels[level].cpuData.clone();
}

void FrameBuffer::getLevelSize(int level, int* width, int* height) const {
    if (!isValidLevel(level)) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }
    
    const BufferLevel& bufferLevel = m_levels[level];
    if (width) *width = bufferLevel.width;
    if (height) *height = bufferLevel.height;
}

void FrameBuffer::setCurrentLevel(int level) {
    if (isValidLevel(level)) {
        m_currentLevel = level;
    }
}

int FrameBuffer::findOptimalPreviewLevel(int maxWidth, int maxHeight) const {
    int bestLevel = -1;
    size_t minDiff = SIZE_MAX;
    
    for (size_t i = 0; i < m_levels.size(); ++i) {
        const BufferLevel& level = m_levels[i];
        
        // 요구 크기보다 큰 레벨 중에서 가장 작은 것 선택
        if (level.width <= maxWidth && level.height <= maxHeight) {
            size_t diff = (maxWidth - level.width) + (maxHeight - level.height);
            if (diff < minDiff) {
                minDiff = diff;
                bestLevel = static_cast<int>(i);
            }
        }
    }
    
    // 적절한 레벨이 없으면 가장 작은 레벨 선택
    if (bestLevel == -1 && !m_levels.empty()) {
        bestLevel = 0;
        size_t minArea = m_levels[0].width * m_levels[0].height;
        
        for (size_t i = 1; i < m_levels.size(); ++i) {
            size_t area = m_levels[i].width * m_levels[i].height;
            if (area < minArea) {
                minArea = area;
                bestLevel = static_cast<int>(i);
            }
        }
    }
    
    return bestLevel;
}

void FrameBuffer::cleanupGPUData() {
    for (auto& level : m_levels) {
        level.gpuTexture = nullptr;
        level.gpuDirty = true;
    }
}

void FrameBuffer::cleanup() {
    for (auto& level : m_levels) {
        if (!level.cpuData.empty()) {
            level.cpuData.release();
        }
        level.gpuTexture = nullptr;
    }
    
    m_levels.clear();
    m_currentLevel = 0;
    m_isInitialized = false;
}

bool FrameBuffer::isValidLevel(int level) const {
    return level >= 0 && level < static_cast<int>(m_levels.size());
}

std::string FrameBuffer::getInfo() const {
    if (!m_isInitialized) {
        return "FrameBuffer not initialized";
    }
    
    std::string info = "FrameBuffer: " + m_name + "\n";
    info += "Levels: " + std::to_string(m_levels.size()) + "\n";
    info += "Current Level: " + std::to_string(m_currentLevel) + "\n";
    
    for (size_t i = 0; i < m_levels.size(); ++i) {
        const BufferLevel& level = m_levels[i];
        info += "Level " + std::to_string(i) + ": " + 
                std::to_string(level.width) + "x" + std::to_string(level.height);
        if (level.gpuTexture) {
            info += " (GPU)";
        }
        info += "\n";
    }
    
    return info;
}
