#pragma once

#include <string>
#include <memory>
#include <opencv2/opencv.hpp>

/**
 * @brief 이미지 데이터와 메타데이터를 관리하는 클래스
 * 
 * 이 클래스는 이미지의 원본 데이터, 텍스처 정보, 메타데이터를
 * 캡슐화하여 백엔드 중립적인 방식으로 이미지를 관리합니다.
 * 
 * 주요 기능:
 * - 이미지 데이터와 메타데이터 저장
 * - 다중 해상도 텍스처 지원
 * - 메모리 효율적인 데이터 관리
 * - 스트리밍 데이터 지원
 */
struct ImageData {
    void* textureId = nullptr;      // GPU 텍스처 ID
    void* previewTextureId = nullptr; // 프리뷰용 저해상도 텍스처 ID
    int width = 0;                  // 원본 너비
    int height = 0;                 // 원본 높이
    int channels = 0;               // 채널 수 (RGB: 3, RGBA: 4)
    int previewWidth = 0;           // 프리뷰 너비
    int previewHeight = 0;          // 프리뷰 높이
    bool isStreaming = false;       // 스트리밍 데이터 여부
    
    ImageData() = default;
    
    ImageData(void* texId, int w, int h, int c) 
        : textureId(texId), width(w), height(h), channels(c) {}
};

class Image {
private:
    ImageData m_data;               // 이미지 데이터
    std::string m_filepath;         // 파일 경로
    bool m_isLoaded = false;        // 로드 상태
    std::string m_name;             // 이미지 이름/설명
    cv::Mat m_cpuData;              // CPU 메모리 데이터 (처리용)
    
public:
    Image() = default;
    
    /**
     * @brief 파일 경로로 이미지 생성
     * @param filepath 이미지 파일 경로
     */
    explicit Image(const std::string& filepath);
    
    /**
     * @brief OpenCV Mat 데이터로 이미지 생성
     * @param mat OpenCV Mat 데이터
     * @param name 이미지 이름
     */
    Image(const cv::Mat& mat, const std::string& name = "");
    
    /**
     * @brief 소멸자 - 리소스 정리
     */
    ~Image();
    
    // Getters
    const ImageData& getData() const { return m_data; }
    const std::string& getFilepath() const { return m_filepath; }
    bool isLoaded() const { return m_isLoaded; }
    const std::string& getName() const { return m_name; }
    const cv::Mat& getCpuData() const { return m_cpuData; }
    
    // Setters
    void setData(const ImageData& data) { m_data = data; m_isLoaded = true; }
    void setFilepath(const std::string& path) { m_filepath = path; }
    void setLoaded(bool loaded) { m_isLoaded = loaded; }
    void setName(const std::string& name) { m_name = name; }
    void setCpuData(const cv::Mat& data) { m_cpuData = data.clone(); }
    
    /**
     * @brief 텍스처 ID 설정
     * @param textureId GPU 텍스처 ID
     */
    void setTextureId(void* textureId);
    
    /**
     * @brief 프리뷰 텍스처 ID 설정
     * @param previewTextureId 프리뷰 텍스처 ID
     * @param previewWidth 프리뷰 너비
     * @param previewHeight 프리뷰 높이
     */
    void setPreviewTextureId(void* previewTextureId, int previewWidth, int previewHeight);
    
    /**
     * @brief 이미지 크기 정보 설정
     * @param width 너비
     * @param height 높이
     * @param channels 채널 수
     */
    void setSize(int width, int height, int channels);
    
    /**
     * @brief 스트리밍 모드 설정
     * @param streaming 스트리밍 여부
     */
    void setStreaming(bool streaming) { m_data.isStreaming = streaming; }
    
    /**
     * @brief 이미지 리소스 정리
     */
    void cleanup();
    
    /**
     * @brief 유효한 이미지인지 확인
     * @return 유효 여부
     */
    bool isValid() const;
    
    /**
     * @brief 이미지 정보 문자열 반환
     * @return 이미지 정보
     */
    std::string getInfo() const;
};
