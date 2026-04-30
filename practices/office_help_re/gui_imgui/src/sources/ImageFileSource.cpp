#include "sources/ImageFileSource.h"
#include <iostream>

bool ImageFileSource::open(const std::string& source) {
    m_filepath = source;
    m_lastError.clear();
    
    // OpenCV로 이미지 로드
    m_image = cv::imread(source, cv::IMREAD_UNCHANGED);
    
    if (m_image.empty()) {
        m_lastError = "Failed to load image: " + source;
        m_isOpen = false;
        return false;
    }
    
    m_isOpen = true;
    return true;
}

bool ImageFileSource::readNextFrame(cv::Mat& frame) {
    if (!m_isOpen || m_image.empty()) {
        m_lastError = "No image loaded";
        return false;
    }
    
    // 정적 이미지는 항상 동일한 프레임 반환
    frame = m_image.clone();
    return true;
}

bool ImageFileSource::readCurrentFrame(cv::Mat& frame) {
    return readNextFrame(frame); // 정적 이미지는 현재 프레임과 다음 프레임이 동일
}

bool ImageFileSource::seekToFrame(int frameIndex) {
    // 정적 이미지는 시킹 의미 없음
    return m_isOpen && !m_image.empty();
}

bool ImageFileSource::isAvailable() const {
    return m_isOpen && !m_image.empty();
}

bool ImageFileSource::isStreaming() const {
    return false; // 정적 이미지는 스트리밍 아님
}

void ImageFileSource::close() {
    m_isOpen = false;
    if (!m_image.empty()) {
        m_image.release();
    }
    m_filepath.clear();
    m_lastError.clear();
}

DataSourceType ImageFileSource::getType() const {
    return DataSourceType::StaticImage;
}

std::string ImageFileSource::getSourceInfo() const {
    if (!m_isOpen || m_image.empty()) {
        return "No image loaded";
    }
    
    std::string info = "Image File: " + m_filepath + "\n";
    info += "Size: " + std::to_string(m_image.cols) + "x" + std::to_string(m_image.rows) + "\n";
    info += "Channels: " + std::to_string(m_image.channels());
    
    return info;
}

int ImageFileSource::getTotalFrames() const {
    return m_isOpen ? 1 : 0; // 정적 이미지는 1 프레임
}

int ImageFileSource::getCurrentFrameIndex() const {
    return m_isOpen ? 0 : -1;
}

double ImageFileSource::getFrameRate() const {
    return 0.0; // 정적 이미지는 프레임레이트 없음
}

void ImageFileSource::getResolution(int* width, int* height) const {
    if (m_isOpen && !m_image.empty()) {
        if (width) *width = m_image.cols;
        if (height) *height = m_image.rows;
    } else {
        if (width) *width = 0;
        if (height) *height = 0;
    }
}

void ImageFileSource::setPlayState(bool play) {
    // 정적 이미지는 재생 상태 의미 없음
}

bool ImageFileSource::isPlaying() const {
    return false;
}

void ImageFileSource::setLooping(bool loop) {
    // 정적 이미지는 루핑 의미 없음
}

bool ImageFileSource::isLooping() const {
    return false;
}

std::string ImageFileSource::getLastError() const {
    return m_lastError;
}

bool ImageFileSource::isInitialized() const {
    return m_isOpen && !m_image.empty();
}
