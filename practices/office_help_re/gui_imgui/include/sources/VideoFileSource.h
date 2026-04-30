#pragma once

#include "core/IDataSource.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

/**
 * @brief 비디오 파일 데이터 소스
 * 
 * 이 클래스는 비디오 파일에서 프레임을 읽어오는
 * 데이터 소스를 구현합니다.
 * 
 * 주요 기능:
 * - 다양한 비디오 포맷 지원
 * - 프레임별 탐색 및 재생
 * - 실시간 및 가속 재생
 * - 비디오 정보 제공 (길이, FPS, 해상도 등)
 */
class VideoFileSource : public IDataSource {
private:
    cv::VideoCapture m_videoCapture;                 // OpenCV 비디오 캡처 객체
    std::string m_videoPath;                         // 비디오 파일 경로
    int m_currentFrame;                               // 현재 프레임 인덱스
    int m_totalFrames;                                // 전체 프레임 수
    double m_fps;                                     // 비디오 FPS
    cv::Size m_frameSize;                            // 프레임 크기
    bool m_isLooping;                                 // 루프 재생 여부
    double m_playbackSpeed;                           // 재생 속도 배율
    bool m_isPlaying;                                 // 재생 상태
    std::chrono::steady_clock::time_point m_lastFrameTime; // 마지막 프레임 시간
    cv::Mat m_currentFrameMat;                        // 현재 프레임 이미지

public:
    /**
     * @brief 생성자
     */
    VideoFileSource();
    
    /**
     * @brief 소멸자
     */
    ~VideoFileSource() override;

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

    // 비디오 특화 메서드
    void setLooping(bool enable);
    bool getLooping() const;
    
    void setPlaybackSpeed(double speed);
    double getPlaybackSpeed() const;
    
    void play();
    void pause();
    void stop();
    bool isPlaying() const;
    
    bool readNextFrameForPlayback(cv::Mat& frame);
    
    // 비디오 정보 메서드
    int getTotalFrames() const;
    double getDuration() const;
    double getFPS() const;
    cv::Size getFrameSize() const;
    int getCodec() const;
    
    // 프레임 탐색 메서드
    bool seekToPercentage(double percentage);
    bool seekToBeginning();
    bool seekToEnd();
    
    bool skipFrames(int frameCount);
    bool skipToNextKeyFrame();
    bool skipToPreviousKeyFrame();
    
    // 비디오 속성 메서드
    double getBitrate() const;
    std::string getVideoFormat() const;
    std::string getContainerFormat() const;
    
    // 프레임 처리 메서드
    cv::Mat getCurrentFrame() const;
    cv::Mat getFrameAt(int frameIndex);
    cv::Mat getFrameAtTime(double timeInSeconds);
    
    // 비디오 분석 메서드
    std::vector<int> getKeyFrameIndices() const;
    bool isKeyFrame(int frameIndex) const;
    
    // 스트리밍 메서드
    bool startStreaming();
    bool stopStreaming();
    bool isStreaming() const;

private:
    /**
     * @brief 비디오 속성 업데이트
     */
    void updateVideoProperties();
    
    /**
     * @brief 프레임 시간 계산
     * @return 다음 프레임까지 대기 시간 (ms)
     */
    double calculateFrameDelay() const;
    
    /**
     * @brief 프레임 인덱스 유효성 검사
     * @param frameIndex 프레임 인덱스
     * @return 유효성 여부
     */
    bool isValidFrameIndex(int frameIndex) const;
    
    /**
     * @brief 시간을 프레임 인덱스로 변환
     * @param timeInSeconds 시간 (초)
     * @return 프레임 인덱스
     */
    int timeToFrameIndex(double timeInSeconds) const;
    
    /**
     * @brief 프레임 인덱스를 시간으로 변환
     * @param frameIndex 프레임 인덱스
     * @return 시간 (초)
     */
    double frameIndexToTime(int frameIndex) const;
    
    /**
     * @brief 비디오 파일 포맷 감지
     * @param filePath 파일 경로
     * @return 포맷 문자열
     */
    std::string detectVideoFormat(const std::string& filePath) const;
    
    /**
     * @brief 컨테이너 포맷 감지
     * @param filePath 파일 경로
     * @return 컨테이너 포맷
     */
    std::string detectContainerFormat(const std::string& filePath) const;
    
    /**
     * @brief 키프레임 인덱스 계산 (간단한 구현)
     * @return 키프레임 인덱스 목록
     */
    std::vector<int> calculateKeyFrameIndices() const;
    
    /**
     * @brief 스트리밍 상태 업데이트
     */
    void updateStreamingState();
    
    /**
     * @brief 프레임 시간 동기화
     * @return 다음 프레임을 읽을 시간인지 여부
     */
    bool shouldReadNextFrame() const;
};
