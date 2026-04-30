#include "image/Image.h"
#include <iostream>

Image::Image(const std::string& filepath) 
    : m_filepath(filepath), m_isLoaded(false) {
    m_name = filepath.substr(filepath.find_last_of("/\\") + 1);
}

Image::Image(const cv::Mat& mat, const std::string& name) 
    : m_name(name), m_isLoaded(false) {
    if (!mat.empty()) {
        m_cpuData = mat.clone();
        setSize(mat.cols, mat.rows, mat.channels());
    }
}

Image::~Image() {
    cleanup();
}

void Image::setTextureId(void* textureId) {
    m_data.textureId = textureId;
    if (textureId) {
        m_isLoaded = true;
    }
}

void Image::setPreviewTextureId(void* previewTextureId, int previewWidth, int previewHeight) {
    m_data.previewTextureId = previewTextureId;
    m_data.previewWidth = previewWidth;
    m_data.previewHeight = previewHeight;
}

void Image::setSize(int width, int height, int channels) {
    m_data.width = width;
    m_data.height = height;
    m_data.channels = channels;
}

void Image::cleanup() {
    // GPU 텍스처 정리는 ImageLoader에서 담당
    // 여기서는 데이터만 초기화
    m_data.textureId = nullptr;
    m_data.previewTextureId = nullptr;
    m_data.width = 0;
    m_data.height = 0;
    m_data.channels = 0;
    m_data.previewWidth = 0;
    m_data.previewHeight = 0;
    m_isLoaded = false;
    
    if (!m_cpuData.empty()) {
        m_cpuData.release();
    }
}

bool Image::isValid() const {
    return m_isLoaded && 
           m_data.textureId != nullptr && 
           m_data.width > 0 && 
           m_data.height > 0 && 
           m_data.channels > 0;
}

std::string Image::getInfo() const {
    if (!m_isLoaded) {
        return "Image not loaded";
    }
    
    std::string info = "Name: " + m_name + "\n";
    info += "Size: " + std::to_string(m_data.width) + "x" + std::to_string(m_data.height) + "\n";
    info += "Channels: " + std::to_string(m_data.channels) + "\n";
    info += "Type: " + (m_data.isStreaming ? "Streaming" : "Static") + "\n";
    
    if (!m_filepath.empty()) {
        info += "Path: " + m_filepath;
    }
    
    return info;
}
