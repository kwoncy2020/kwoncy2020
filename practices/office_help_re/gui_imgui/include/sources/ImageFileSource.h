#pragma once

#include "core/IDataSource.h"
#include <string>

/**
 * @brief 정적 이미지 파일 데이터 소스
 * 
 * 이 클래스는 이미지 파일에서 데이터를 읽어오는 IDataSource 구현입니다.
 * 한 번 로드된 이미지를 반복적으로 제공합니다.
 * 
 * 주요 기능:
 * - 다양한 이미지 포맷 지원 (JPEG, PNG, BMP, TIFF 등)
 * - OpenCV를 통한 이미지 로딩
 * - 에러 핸들링 및 상태 관리
 */
class ImageFileSource : public IDataSource {
private:
    std::string m_filepath;        // 이미지 파일 경로
    cv::Mat m_image;               // 로드된 이미지 데이터
    bool m_isOpen = false;         // 열림 상태
    std::string m_lastError;       // 마지막 에러 메시지

public:
    /**
     * @brief 생성자
     */
    ImageFileSource() = default;
    
    /**
     * @brief 소멸자
     */
    ~ImageFileSource() override = default;

    // IDataSource 인터페이스 구현
    bool open(const std::string& source) override;
    bool readNextFrame(cv::Mat& frame) override;
    bool readCurrentFrame(cv::Mat& frame) override;
    bool seekToFrame(int frameIndex) override;
    bool isAvailable() const override;
    bool isStreaming() const override;
    void close() override;
    DataSourceType getType() const override;
    std::string getSourceInfo() const override;
    int getTotalFrames() const override;
    int getCurrentFrameIndex() const override;
    double getFrameRate() const override;
    void getResolution(int* width, int* height) const override;
    void setPlayState(bool play) override;
    bool isPlaying() const override;
    void setLooping(bool loop) override;
    bool isLooping() const override;
    std::string getLastError() const override;
    bool isInitialized() const override;

    /**
     * @brief 파일 경로 가져오기
     * @return 파일 경로
     */
    const std::string& getFilepath() const { return m_filepath; }
    
    /**
     * @brief 이미지 데이터 직접 가져오기
     * @return OpenCV Mat 이미지 데이터
     */
    const cv::Mat& getImage() const { return m_image; }
};
