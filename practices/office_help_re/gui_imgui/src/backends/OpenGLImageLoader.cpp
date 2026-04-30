#include "backends/OpenGLImageLoader.h"
#include "imgui.h"
#include <iostream>
#include <chrono>

OpenGLImageLoader::OpenGLImageLoader() : m_initialized(false) {
}

OpenGLImageLoader::~OpenGLImageLoader() {
    cleanup();
}

bool OpenGLImageLoader::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // OpenGL 컨텍스트가 있는지 확인
    if (!glGetString(GL_VERSION)) {
        std::cerr << "OpenGL context not available" << std::endl;
        return false;
    }
    
    m_initialized = true;
    return true;
}

void* OpenGLImageLoader::loadImageFromFile(const std::string& filepath) {
    if (!m_initialized && !initialize()) {
        return nullptr;
    }
    
    // OpenCV로 이미지 로드
    cv::Mat image = cv::imread(filepath, cv::IMREAD_UNCHANGED);
    if (image.empty()) {
        std::cerr << "Failed to load image: " << filepath << std::endl;
        return nullptr;
    }
    
    int width, height;
    GLuint textureId = convertMatToTexture(image);
    if (textureId == 0) {
        return nullptr;
    }
    
    addTextureInfo(textureId, image.cols, image.rows, image.channels(), false);
    return reinterpret_cast<void*>(textureId);
}

void* OpenGLImageLoader::createTextureFromMat(const cv::Mat& mat, int* width, int* height) {
    if (!m_initialized && !initialize()) {
        return nullptr;
    }
    
    if (mat.empty()) {
        return nullptr;
    }
    
    GLuint textureId = convertMatToTexture(mat);
    if (textureId == 0) {
        return nullptr;
    }
    
    addTextureInfo(textureId, mat.cols, mat.rows, mat.channels(), false);
    
    if (width) *width = mat.cols;
    if (height) *height = mat.rows;
    
    return reinterpret_cast<void*>(textureId);
}

bool OpenGLImageLoader::updateTextureFromFrame(void* textureId, const cv::Mat& frame) {
    if (!m_initialized || !textureId || frame.empty()) {
        return false;
    }
    
    GLuint glTextureId = reinterpret_cast<GLuint>(textureId);
    std::lock_guard<std::mutex> lock(m_textureMutex);
    
    TextureInfo* info = findTextureInfo(glTextureId);
    if (!info) {
        std::cerr << "Texture not found for update" << std::endl;
        return false;
    }
    
    // 크기가 다르면 텍스처 재생성
    if (frame.cols != info->width || frame.rows != info->height) {
        // 기존 텍스처 삭제
        glDeleteTextures(1, &glTextureId);
        
        // 새로운 텍스처 생성
        GLuint newTextureId = convertMatToTexture(frame);
        if (newTextureId == 0) {
            return false;
        }
        
        // 텍스처 정보 업데이트
        info->textureId = newTextureId;
        info->width = frame.cols;
        info->height = frame.rows;
        info->channels = frame.channels();
        info->lastUsedTime = getCurrentTimeMillis();
        
        return true;
    }
    
    // 기존 텍스처에 데이터 업데이트
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    
    cv::Mat convertedFrame;
    GLenum format = convertPixelFormat(frame.type());
    GLenum internalFormat = determineInternalFormat(frame.channels());
    
    if (frame.channels() == 3) {
        cv::cvtColor(frame, convertedFrame, cv::COLOR_BGR2RGB);
    } else if (frame.channels() == 4) {
        cv::cvtColor(frame, convertedFrame, cv::COLOR_BGRA2RGBA);
    } else {
        convertedFrame = frame;
    }
    
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.cols, frame.rows, 
                   format, GL_UNSIGNED_BYTE, convertedFrame.data);
    
    info->lastUsedTime = getCurrentTimeMillis();
    
    return checkGLError("updateTextureFromFrame");
}

void* OpenGLImageLoader::createStreamingTexture() {
    if (!m_initialized && !initialize()) {
        return nullptr;
    }
    
    // 스트리밍용 빈 텍스처 생성 (나중에 크기가 정해짐)
    GLuint textureId = createGLTexture(1, 1, nullptr);
    if (textureId == 0) {
        return nullptr;
    }
    
    addTextureInfo(textureId, 1, 1, 4, true); // 임시 크기
    return reinterpret_cast<void*>(textureId);
}

void* OpenGLImageLoader::createPreviewTexture(const cv::Mat& frame, int previewWidth, int previewHeight) {
    if (!m_initialized && !initialize()) {
        return nullptr;
    }
    
    if (frame.empty()) {
        return nullptr;
    }
    
    // 프리뷰 크기로 리사이즈
    cv::Mat resizedFrame;
    cv::resize(frame, resizedFrame, cv::Size(previewWidth, previewHeight));
    
    GLuint textureId = convertMatToTexture(resizedFrame);
    if (textureId == 0) {
        return nullptr;
    }
    
    addTextureInfo(textureId, previewWidth, previewHeight, resizedFrame.channels(), false);
    return reinterpret_cast<void*>(textureId);
}

void OpenGLImageLoader::getTextureSize(void* textureId, int* width, int* height) {
    if (!textureId) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_textureMutex);
    const TextureInfo* info = findTextureInfo(reinterpret_cast<GLuint>(textureId));
    if (info) {
        if (width) *width = info->width;
        if (height) *height = info->height;
    } else {
        if (width) *width = 0;
        if (height) *height = 0;
    }
}

void OpenGLImageLoader::destroyTexture(void* textureId) {
    if (!textureId) {
        return;
    }
    
    GLuint glTextureId = reinterpret_cast<GLuint>(textureId);
    std::lock_guard<std::mutex> lock(m_textureMutex);
    
    TextureInfo* info = findTextureInfo(glTextureId);
    if (info) {
        glDeleteTextures(1, &glTextureId);
        removeTextureInfo(glTextureId);
        checkGLError("destroyTexture");
    }
}

void OpenGLImageLoader::cleanup() {
    std::lock_guard<std::mutex> lock(m_textureMutex);
    
    for (auto& pair : m_textureMap) {
        GLuint textureId = reinterpret_cast<GLuint>(pair.first);
        glDeleteTextures(1, &textureId);
    }
    
    m_textureMap.clear();
    m_currentTextureCount = 0;
}

bool OpenGLImageLoader::isInitialized() const {
    return m_initialized;
}

void OpenGLImageLoader::setTextureFilters(GLint minFilter, GLint magFilter) {
    m_minFilter = minFilter;
    m_magFilter = magFilter;
}

void OpenGLImageLoader::setTextureWrap(GLint wrapS, GLint wrapT) {
    m_wrapS = wrapS;
    m_wrapT = wrapT;
}

void OpenGLImageLoader::setMaxTextures(size_t maxTextures) {
    m_maxTextures = maxTextures;
}

size_t OpenGLImageLoader::getCurrentTextureCount() const {
    std::lock_guard<std::mutex> lock(m_textureMutex);
    return m_currentTextureCount;
}

const OpenGLImageLoader::TextureInfo* OpenGLImageLoader::getTextureInfo(void* textureId) const {
    if (!textureId) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(m_textureMutex);
    return findTextureInfo(reinterpret_cast<GLuint>(textureId));
}

size_t OpenGLImageLoader::cleanupUnusedTextures(size_t maxAge) {
    std::lock_guard<std::mutex> lock(m_textureMutex);
    
    size_t currentTime = getCurrentTimeMillis();
    size_t cleanedCount = 0;
    
    auto it = m_textureMap.begin();
    while (it != m_textureMap.end()) {
        if (currentTime - it->second.lastUsedTime > maxAge) {
            GLuint textureId = reinterpret_cast<GLuint>(it->first);
            glDeleteTextures(1, &textureId);
            it = m_textureMap.erase(it);
            cleanedCount++;
            m_currentTextureCount--;
        } else {
            ++it;
        }
    }
    
    return cleanedCount;
}

size_t OpenGLImageLoader::estimateMemoryUsage() const {
    std::lock_guard<std::mutex> lock(m_textureMutex);
    
    size_t totalMemory = 0;
    for (const auto& pair : m_textureMap) {
        const TextureInfo& info = pair.second;
        // RGBA 4바이트 * 너비 * 높이
        totalMemory += info.width * info.height * 4;
    }
    
    return totalMemory;
}

bool OpenGLImageLoader::checkGLError(const std::string& operation) const {
    GLenum error;
    bool hasError = false;
    
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL Error in " << operation << ": " << error << std::endl;
        hasError = true;
    }
    
    return !hasError;
}

// Private methods

GLuint OpenGLImageLoader::createGLTexture(int width, int height, const void* data, GLenum format) {
    GLuint textureId;
    glGenTextures(1, &textureId);
    
    if (textureId == 0) {
        checkGLError("glGenTextures");
        return 0;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
    
    GLenum internalFormat = determineInternalFormat(4); // 기본 RGBA
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, 
                format, GL_UNSIGNED_BYTE, data);
    
    if (checkGLError("createGLTexture")) {
        glDeleteTextures(1, &textureId);
        return 0;
    }
    
    return textureId;
}

GLuint OpenGLImageLoader::convertMatToTexture(const cv::Mat& mat, GLuint targetTextureId) {
    if (mat.empty()) {
        return 0;
    }
    
    cv::Mat convertedMat;
    
    // OpenCV BGR/BGRA를 OpenGL RGB/RGBA로 변환
    if (mat.channels() == 3) {
        cv::cvtColor(mat, convertedMat, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 4) {
        cv::cvtColor(mat, convertedMat, cv::COLOR_BGRA2RGBA);
    } else if (mat.channels() == 1) {
        cv::cvtColor(mat, convertedMat, cv::COLOR_GRAY2RGB);
    } else {
        convertedMat = mat;
    }
    
    GLenum format = convertPixelFormat(convertedMat.type());
    GLenum internalFormat = determineInternalFormat(convertedMat.channels());
    
    GLuint textureId;
    if (targetTextureId != 0) {
        textureId = targetTextureId;
        glBindTexture(GL_TEXTURE_2D, textureId);
    } else {
        textureId = createGLTexture(convertedMat.cols, convertedMat.rows, 
                                  convertedMat.data, format);
        if (textureId == 0) {
            return 0;
        }
    }
    
    if (targetTextureId != 0) {
        // 기존 텍스처 업데이트
        if (convertedMat.cols != 0 && convertedMat.rows != 0) {
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, convertedMat.cols, convertedMat.rows, 0,
                        format, GL_UNSIGNED_BYTE, convertedMat.data);
        }
    }
    
    return textureId;
}

void OpenGLImageLoader::addTextureInfo(GLuint textureId, int width, int height, int channels, bool isStreaming) {
    std::lock_guard<std::mutex> lock(m_textureMutex);
    
    void* key = reinterpret_cast<void*>(textureId);
    TextureInfo info;
    info.textureId = textureId;
    info.width = width;
    info.height = height;
    info.channels = channels;
    info.isStreaming = isStreaming;
    info.lastUsedTime = getCurrentTimeMillis();
    
    m_textureMap[key] = info;
    m_currentTextureCount++;
    
    // 최대 텍스처 수 초과 시 오래된 텍스처 정리
    if (m_currentTextureCount > m_maxTextures) {
        cleanupUnusedTextures(30000); // 30초 이상 사용하지 않은 텍스처 정리
    }
}

void OpenGLImageLoader::removeTextureInfo(GLuint textureId) {
    void* key = reinterpret_cast<void*>(textureId);
    auto it = m_textureMap.find(key);
    if (it != m_textureMap.end()) {
        m_textureMap.erase(it);
        m_currentTextureCount--;
    }
}

OpenGLImageLoader::TextureInfo* OpenGLImageLoader::findTextureInfo(GLuint textureId) {
    void* key = reinterpret_cast<void*>(textureId);
    auto it = m_textureMap.find(key);
    return (it != m_textureMap.end()) ? &it->second : nullptr;
}

const OpenGLImageLoader::TextureInfo* OpenGLImageLoader::findTextureInfo(GLuint textureId) const {
    void* key = reinterpret_cast<void*>(textureId);
    auto it = m_textureMap.find(key);
    return (it != m_textureMap.end()) ? &it->second : nullptr;
}

bool OpenGLImageLoader::isTextureSizeSupported(int width, int height) const {
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    return width <= maxTextureSize && height <= maxTextureSize;
}

GLenum OpenGLImageLoader::convertPixelFormat(int cvFormat) const {
    switch (cvFormat) {
        case CV_8UC1: return GL_RED;
        case CV_8UC3: return GL_RGB;
        case CV_8UC4: return GL_RGBA;
        default: return GL_RGBA;
    }
}

GLenum OpenGLImageLoader::determineInternalFormat(int channels) const {
    switch (channels) {
        case 1: return GL_R8;
        case 2: return GL_RG8;
        case 3: return GL_RGB8;
        case 4: return GL_RGBA8;
        default: return GL_RGBA8;
    }
}

size_t OpenGLImageLoader::getCurrentTimeMillis() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
