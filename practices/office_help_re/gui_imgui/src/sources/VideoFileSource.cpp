#include "sources/VideoFileSource.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <filesystem>

VideoFileSource::VideoFileSource() 
    : m_currentFrame(0), m_totalFrames(0), m_fps(30.0),
      m_frameSize(0, 0), m_isLooping(false), m_playbackSpeed(1.0),
      m_isPlaying(false) {
}

VideoFileSource::~VideoFileSource() {
    close();
}

bool VideoFileSource::open(const std::string& source) {
    m_videoPath = source;
    m_lastError.clear();
    
    // 비디오 파일 열기
    if (!m_videoCapture.open(source)) {
        m_lastError = "Failed to open video file: " + source;
        m_isOpen = false;
        return false;
    }
    
    // 비디오 속성 업데이트
    updateVideoProperties();
    
    if (m_totalFrames <= 0) {
        m_lastError = "Invalid video: no frames found";
        m_videoCapture.release();
        m_isOpen = false;
        return false;
    }
    
    m_currentFrame = 0;
    m_isOpen = true;
    m_lastFrameTime = std::chrono::steady_clock::now();
    
    std::cout << "Opened video: " << source 
              << " (" << m_frameSize.width << "x" << m_frameSize.height 
              << ", " << m_totalFrames << " frames, " << m_fps << " FPS)" << std::endl;
    
    return true;
}

bool VideoFileSource::close() {
    if (m_videoCapture.isOpened()) {
        m_videoCapture.release();
    }
    
    m_currentFrameMat.release();
    m_isOpen = false;
    m_isPlaying = false;
    m_currentFrame = 0;
    
    std::cout << "Closed video: " << m_videoPath << std::endl;
    return true;
}

bool VideoFileSource::isOpen() const {
    return m_isOpen && m_videoCapture.isOpened();
}

bool VideoFileSource::readNextFrame(cv::Mat& frame) {
    if (!isOpen()) {
        m_lastError = "Video source is not open";
        return false;
    }
    
    if (!m_videoCapture.read(frame)) {
        if (m_isLooping) {
            // 루프 재생: 처음으로 돌아가기
            seekToFrame(0);
            return m_videoCapture.read(frame);
        } else {
            m_lastError = "No more frames or end of video";
            return false;
        }
    }
    
    m_currentFrame++;
    m_currentFrameMat = frame.clone();
    return true;
}

bool VideoFileSource::readFrameAt(int frameIndex, cv::Mat& frame) {
    if (!isValidFrameIndex(frameIndex)) {
        m_lastError = "Invalid frame index: " + std::to_string(frameIndex);
        return false;
    }
    
    // 현재 위치 저장
    int currentPos = m_videoCapture.get(cv::CAP_PROP_POS_FRAMES);
    
    // 지정된 프레임으로 이동
    if (!seekToFrame(frameIndex)) {
        return false;
    }
    
    // 프레임 읽기
    bool success = m_videoCapture.read(frame);
    
    // 원래 위치로 복원
    seekToFrame(currentPos);
    
    if (success) {
        m_currentFrameMat = frame.clone();
    }
    
    return success;
}

bool VideoFileSource::seekToFrame(int frameIndex) {
    if (!isOpen()) {
        m_lastError = "Video source is not open";
        return false;
    }
    
    if (!isValidFrameIndex(frameIndex)) {
        m_lastError = "Invalid frame index: " + std::to_string(frameIndex);
        return false;
    }
    
    if (!m_videoCapture.set(cv::CAP_PROP_POS_FRAMES, frameIndex)) {
        m_lastError = "Failed to seek to frame: " + std::to_string(frameIndex);
        return false;
    }
    
    m_currentFrame = frameIndex;
    return true;
}

bool VideoFileSource::seekToTime(double timeInSeconds) {
    int frameIndex = timeToFrameIndex(timeInSeconds);
    return seekToFrame(frameIndex);
}

DataSourceInfo VideoFileSource::getSourceInfo() const {
    DataSourceInfo info;
    info.sourceType = "Video File";
    info.sourcePath = m_videoPath;
    info.isLive = false;
    info.width = m_frameSize.width;
    info.height = m_frameSize.height;
    info.channels = 3; // 대부분의 비디오는 컬러
    info.fps = m_fps;
    info.totalFrames = m_totalFrames;
    info.duration = frameIndexToTime(m_totalFrames);
    info.currentFrame = m_currentFrame;
    info.currentTime = frameIndexToTime(m_currentFrame);
    info.canSeek = true;
    info.canPause = true;
    info.canRewind = true;
    info.supportsRealTime = true;
    
    return info;
}

std::string VideoFileSource::getLastError() const {
    return m_lastError;
}

bool VideoFileSource::isLive() const {
    return false;
}

int VideoFileSource::getCurrentFrameIndex() const {
    return m_currentFrame;
}

double VideoFileSource::getCurrentTime() const {
    return frameIndexToTime(m_currentFrame);
}

bool VideoFileSource::hasMoreFrames() const {
    if (!isOpen()) {
        return false;
    }
    
    return m_isLooping || (m_currentFrame < m_totalFrames);
}

void VideoFileSource::reset() {
    seekToFrame(0);
    m_currentFrame = 0;
    m_lastFrameTime = std::chrono::steady_clock::now();
}

// 비디오 특화 메서드

void VideoFileSource::setLooping(bool enable) {
    m_isLooping = enable;
}

bool VideoFileSource::getLooping() const {
    return m_isLooping;
}

void VideoFileSource::setPlaybackSpeed(double speed) {
    m_playbackSpeed = std::max(0.1, std::min(10.0, speed));
}

double VideoFileSource::getPlaybackSpeed() const {
    return m_playbackSpeed;
}

void VideoFileSource::play() {
    m_isPlaying = true;
    m_lastFrameTime = std::chrono::steady_clock::now();
}

void VideoFileSource::pause() {
    m_isPlaying = false;
}

void VideoFileSource::stop() {
    m_isPlaying = false;
    seekToFrame(0);
}

bool VideoFileSource::isPlaying() const {
    return m_isPlaying;
}

bool VideoFileSource::readNextFrameForPlayback(cv::Mat& frame) {
    if (!m_isPlaying) {
        return false;
    }
    
    // 재생 속도에 따른 프레임 타이밍 체크
    if (!shouldReadNextFrame()) {
        return false;
    }
    
    bool success = readNextFrame(frame);
    if (success) {
        m_lastFrameTime = std::chrono::steady_clock::now();
    }
    
    return success;
}

// 비디오 정보 메서드

int VideoFileSource::getTotalFrames() const {
    return m_totalFrames;
}

double VideoFileSource::getDuration() const {
    return frameIndexToTime(m_totalFrames);
}

double VideoFileSource::getFPS() const {
    return m_fps;
}

cv::Size VideoFileSource::getFrameSize() const {
    return m_frameSize;
}

int VideoFileSource::getCodec() const {
    if (!isOpen()) {
        return -1;
    }
    
    return static_cast<int>(m_videoCapture.get(cv::CAP_PROP_FOURCC));
}

// 프레임 탐색 메서드

bool VideoFileSource::seekToPercentage(double percentage) {
    if (percentage < 0.0 || percentage > 100.0) {
        m_lastError = "Invalid percentage: " + std::to_string(percentage);
        return false;
    }
    
    int frameIndex = static_cast<int>((percentage / 100.0) * m_totalFrames);
    return seekToFrame(frameIndex);
}

bool VideoFileSource::seekToBeginning() {
    return seekToFrame(0);
}

bool VideoFileSource::seekToEnd() {
    return seekToFrame(m_totalFrames - 1);
}

bool VideoFileSource::skipFrames(int frameCount) {
    int targetFrame = m_currentFrame + frameCount;
    return seekToFrame(targetFrame);
}

bool VideoFileSource::skipToNextKeyFrame() {
    std::vector<int> keyFrames = getKeyFrameIndices();
    
    auto it = std::upper_bound(keyFrames.begin(), keyFrames.end(), m_currentFrame);
    if (it != keyFrames.end()) {
        return seekToFrame(*it);
    }
    
    return false;
}

bool VideoFileSource::skipToPreviousKeyFrame() {
    std::vector<int> keyFrames = getKeyFrameIndices();
    
    auto it = std::lower_bound(keyFrames.begin(), keyFrames.end(), m_currentFrame);
    if (it != keyFrames.begin()) {
        --it;
        return seekToFrame(*it);
    }
    
    return false;
}

// 비디오 속성 메서드

double VideoFileSource::getBitrate() const {
    if (!isOpen()) {
        return 0.0;
    }
    
    return m_videoCapture.get(cv::CAP_PROP_BITRATE);
}

std::string VideoFileSource::getVideoFormat() const {
    return detectVideoFormat(m_videoPath);
}

std::string VideoFileSource::getContainerFormat() const {
    return detectContainerFormat(m_videoPath);
}

// 프레임 처리 메서드

cv::Mat VideoFileSource::getCurrentFrame() const {
    return m_currentFrameMat.clone();
}

cv::Mat VideoFileSource::getFrameAt(int frameIndex) {
    cv::Mat frame;
    readFrameAt(frameIndex, frame);
    return frame;
}

cv::Mat VideoFileSource::getFrameAtTime(double timeInSeconds) {
    int frameIndex = timeToFrameIndex(timeInSeconds);
    return getFrameAt(frameIndex);
}

// 비디오 분석 메서드

std::vector<int> VideoFileSource::getKeyFrameIndices() const {
    return calculateKeyFrameIndices();
}

bool VideoFileSource::isKeyFrame(int frameIndex) const {
    std::vector<int> keyFrames = getKeyFrameIndices();
    return std::find(keyFrames.begin(), keyFrames.end(), frameIndex) != keyFrames.end();
}

// 스트리밍 메서드

bool VideoFileSource::startStreaming() {
    play();
    return true;
}

bool VideoFileSource::stopStreaming() {
    pause();
    return true;
}

bool VideoFileSource::isStreaming() const {
    return m_isPlaying;
}

// Private 메서드

void VideoFileSource::updateVideoProperties() {
    if (!m_videoCapture.isOpened()) {
        return;
    }
    
    m_totalFrames = static_cast<int>(m_videoCapture.get(cv::CAP_PROP_FRAME_COUNT));
    m_fps = m_videoCapture.get(cv::CAP_PROP_FPS);
    
    int width = static_cast<int>(m_videoCapture.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(m_videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT));
    m_frameSize = cv::Size(width, height);
    
    // FPS가 0이면 기본값 설정
    if (m_fps <= 0.0) {
        m_fps = 30.0;
    }
}

double VideoFileSource::calculateFrameDelay() const {
    if (m_fps <= 0.0) {
        return 33.33; // 30 FPS 기본값
    }
    
    return (1000.0 / m_fps) / m_playbackSpeed; // ms 단위
}

bool VideoFileSource::isValidFrameIndex(int frameIndex) const {
    return frameIndex >= 0 && frameIndex < m_totalFrames;
}

int VideoFileSource::timeToFrameIndex(double timeInSeconds) const {
    if (m_fps <= 0.0) {
        return 0;
    }
    
    int frameIndex = static_cast<int>(timeInSeconds * m_fps);
    return std::max(0, std::min(frameIndex, m_totalFrames - 1));
}

double VideoFileSource::frameIndexToTime(int frameIndex) const {
    if (m_fps <= 0.0) {
        return 0.0;
    }
    
    return static_cast<double>(frameIndex) / m_fps;
}

std::string VideoFileSource::detectVideoFormat(const std::string& filePath) const {
    std::filesystem::path path(filePath);
    std::string extension = path.extension().string();
    
    // 확장자 기반 포맷 감지
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == ".mp4") return "MP4";
    if (extension == ".avi") return "AVI";
    if (extension == ".mov") return "MOV";
    if (extension == ".mkv") return "MKV";
    if (extension == ".wmv") return "WMV";
    if (extension == ".flv") return "FLV";
    if (extension == ".webm") return "WebM";
    if (extension == ".m4v") return "M4V";
    if (extension == ".3gp") return "3GP";
    
    return "Unknown";
}

std::string VideoFileSource::detectContainerFormat(const std::string& filePath) const {
    return detectVideoFormat(filePath); // 현재는 동일하게 처리
}

std::vector<int> VideoFileSource::calculateKeyFrameIndices() const {
    std::vector<int> keyFrames;
    
    // 간단한 키프레임 계산: 1초 간격
    int keyFrameInterval = static_cast<int>(m_fps);
    
    for (int i = 0; i < m_totalFrames; i += keyFrameInterval) {
        keyFrames.push_back(i);
    }
    
    // 마지막 프레임 추가
    if (keyFrames.empty() || keyFrames.back() != m_totalFrames - 1) {
        keyFrames.push_back(m_totalFrames - 1);
    }
    
    return keyFrames;
}

void VideoFileSource::updateStreamingState() {
    // 스트리밍 상태 업데이트 로직
    // 필요한 경우 추가 구현
}

bool VideoFileSource::shouldReadNextFrame() const {
    auto currentTime = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastFrameTime);
    
    double frameDelay = calculateFrameDelay();
    return elapsed.count() >= frameDelay;
}
