#pragma once

#include "image/IImageLoader.h"
#include <unordered_map>
#include <mutex>

/**
 * @brief OpenGL 백엔드 이미지 로더 구현
 * 
 * 이 클래스는 IImageLoader 인터페이스의 OpenGL 구현을 제공합니다.
 * OpenGL 텍스처를 효율적으로 관리하고 성능을 최적화합니다.
 * 
 * 주요 기능:
 * - OpenGL 텍스처 생성 및 관리
 * - 텍스처 재사용으로 성능 최적화
 * - 메모리 누수 방지
 * - 다양한 이미지 포맷 지원
 */
class OpenGLImageLoader : public IImageLoader {
private:
    /**
     * @brief 텍스처 정보 구조체
     */
    struct TextureInfo {
        GLuint textureId;        // OpenGL 텍스처 ID
        int width;               // 너비
        int height;              // 높이
        int channels;            // 채널 수
        bool isStreaming;        // 스트리밍용 여부
        size_t lastUsedTime;     // 마지막 사용 시간 (LRU용)
        
        TextureInfo() : textureId(0), width(0), height(0), channels(0), isStreaming(false), lastUsedTime(0) {}
    };

    std::unordered_map<void*, TextureInfo> m_textureMap;  // 텍스처 맵
    std::mutex m_textureMutex;                           // 텍스처 맵 동기화용 뮤텍스
    bool m_initialized = false;                          // 초기화 상태
    size_t m_maxTextures = 1000;                         // 최대 텍스처 수
    size_t m_currentTextureCount = 0;                    // 현재 텍스처 수
    
    // 텍스처 설정 파라미터
    GLint m_minFilter = GL_LINEAR;                        // 최소화 필터
    GLint m_magFilter = GL_LINEAR;                        // 확대 필터
    GLint m_wrapS = GL_CLAMP_TO_EDGE;                     // S축 랩핑
    GLint m_wrapT = GL_CLAMP_TO_EDGE;                     // T축 랩핑

public:
    /**
     * @brief 생성자
     */
    OpenGLImageLoader();
    
    /**
     * @brief 소멸자
     */
    ~OpenGLImageLoader() override;

    // IImageLoader 인터페이스 구현
    void* loadImageFromFile(const std::string& filepath) override;
    void* createTextureFromMat(const cv::Mat& mat, int* width, int* height) override;
    bool updateTextureFromFrame(void* textureId, const cv::Mat& frame) override;
    void* createStreamingTexture() override;
    void* createPreviewTexture(const cv::Mat& frame, int previewWidth, int previewHeight) override;
    void getTextureSize(void* textureId, int* width, int* height) override;
    void destroyTexture(void* textureId) override;
    void cleanup() override;
    bool isInitialized() const override;

    /**
     * @brief OpenGL 초기화
     * @return 성공 여부
     */
    bool initialize();

    /**
     * @brief 텍스처 필터링 설정
     * @param minFilter 최소화 필터
     * @param magFilter 확대 필터
     */
    void setTextureFilters(GLint minFilter, GLint magFilter);

    /**
     * @brief 텍스처 랩핑 설정
     * @param wrapS S축 랩핑 모드
     * @param wrapT T축 랩핑 모드
     */
    void setTextureWrap(GLint wrapS, GLint wrapT);

    /**
     * @brief 최대 텍스처 수 설정
     * @param maxTextures 최대 텍스처 수
     */
    void setMaxTextures(size_t maxTextures);

    /**
     * @brief 현재 텍스처 수 가져오기
     * @return 현재 텍스처 수
     */
    size_t getCurrentTextureCount() const;

    /**
     * @brief 텍스처 정보 가져오기
     * @param textureId 텍스처 ID
     * @return 텍스처 정보 (없으면 nullptr)
     */
    const TextureInfo* getTextureInfo(void* textureId) const;

    /**
     * @brief 사용하지 않는 텍스처 정리 (LRU)
     * @param maxAge 최대 사용하지 않은 시간 (밀리초)
     * @return 정리된 텍스처 수
     */
    size_t cleanupUnusedTextures(size_t maxAge = 60000); // 기본 1분

    /**
     * @brief 텍스처 메모리 사용량 추정
     * @return 메모리 사용량 (바이트)
     */
    size_t estimateMemoryUsage() const;

    /**
     * @brief OpenGL 에러 체크
     * @param operation 에러 발생 위치 설명
     * @return 에러 발생 여부
     */
    bool checkGLError(const std::string& operation) const;

private:
    /**
     * @brief OpenGL 텍스처 생성
     * @param width 너비
     * @param height 높이
     * @param data 픽셀 데이터 (null이면 빈 텍스처 생성)
     * @param format 픽셀 포맷
     * @return 생성된 텍스처 ID
     */
    GLuint createGLTexture(int width, int height, const void* data = nullptr, GLenum format = GL_RGBA);

    /**
     * @brief OpenCV Mat을 OpenGL 텍스처로 변환
     * @param mat OpenCV Mat 데이터
     * @param targetTextureId 기존 텍스처 ID (0이면 새로 생성)
     * @return 텍스처 ID
     */
    GLuint convertMatToTexture(const cv::Mat& mat, GLuint targetTextureId = 0);

    /**
     * @brief 텍스처 맵에 텍스처 정보 추가
     * @param textureId 텍스처 ID
     * @param width 너비
     * @param height 높이
     * @param channels 채널 수
     * @param isStreaming 스트리밍 여부
     */
    void addTextureInfo(GLuint textureId, int width, int height, int channels, bool isStreaming = false);

    /**
     * @brief 텍스처 맵에서 정보 제거
     * @param textureId 텍스처 ID
     */
    void removeTextureInfo(GLuint textureId);

    /**
     * @brief 텍스처 정보 찾기
     * @param textureId 텍스처 ID
     * @return 텍스처 정보 포인터 (없으면 nullptr)
     */
    TextureInfo* findTextureInfo(GLuint textureId);

    /**
     * @brief 텍스처 정보 찾기 (상수 버전)
     * @param textureId 텍스처 ID
     * @return 텍스처 정보 포인터 (없으면 nullptr)
     */
    const TextureInfo* findTextureInfo(GLuint textureId) const;

    /**
     * @brief 텍스처 최대 크기 확인
     * @param width 너비
     * @param height 높이
     * @return 지원 가능 여부
     */
    bool isTextureSizeSupported(int width, int height) const;

    /**
     * @brief 픽셀 포맷 변환 (OpenCV -> OpenGL)
     * @param cvFormat OpenCV 포맷
     * @return OpenGL 포맷
     */
    GLenum convertPixelFormat(int cvFormat) const;

    /**
     * @brief 내부 포맷 결정
     * @param channels 채널 수
     * @return OpenGL 내부 포맷
     */
    GLenum determineInternalFormat(int channels) const;
};
