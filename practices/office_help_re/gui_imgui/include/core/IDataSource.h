#pragma once

#include <string>
#include <opencv2/opencv.hpp>

/**
 * @brief 데이터 소스 타입 열거형
 * 
 * 애플리케이션에서 지원하는 다양한 입력 소스 타입을 정의합니다.
 */
enum class DataSourceType {
    StaticImage,        // 정적 이미지 파일
    VideoFile,          // 비디오 파일
    Camera,             // 카메라 장치
    WindowCapture,      // 윈도우 화면 캡처
    NetworkStream,      // 네트워크 스트림
    ImageSequence       // 이미지 시퀀스
};

/**
 * @brief 데이터 소스 인터페이스
 * 
 * 이 인터페이스는 파일, 비디오, 카메라, 윈도우 캡처 등
 * 다양한 입력 소스를 통합된 방식으로 처리하기 위한 공통 API를 제공합니다.
 * 
 * 주요 기능:
 * - 다양한 데이터 소스 지원
 * - 실시간 스트리밍 처리
 * - 에러 핸들링
 * - 리소스 관리
 */
class IDataSource {
public:
    virtual ~IDataSource() = default;

    /**
     * @brief 데이터 소스 열기
     * @param source 소스 경로 또는 식별자
     * @return 성공 여부
     */
    virtual bool open(const std::string& source) = 0;

    /**
     * @brief 다음 프레임 읽기
     * @param frame 출력될 프레임 데이터
     * @return 성공 여부 (더 이상 프레임이 없으면 false)
     */
    virtual bool readNextFrame(cv::Mat& frame) = 0;

    /**
     * @brief 현재 프레임 다시 읽기
     * @param frame 출력될 프레임 데이터
     * @return 성공 여부
     */
    virtual bool readCurrentFrame(cv::Mat& frame) = 0;

    /**
     * @brief 특정 프레임으로 이동 (비디오용)
     * @param frameIndex 프레임 인덱스
     * @return 성공 여부
     */
    virtual bool seekToFrame(int frameIndex) = 0;

    /**
     * @brief 데이터 소스 사용 가능 여부 확인
     * @return 사용 가능 여부
     */
    virtual bool isAvailable() const = 0;

    /**
     * @brief 스트리밍 가능 여부 확인
     * @return 스트리밍 가능 여부
     */
    virtual bool isStreaming() const = 0;

    /**
     * @brief 데이터 소스 닫기
     */
    virtual void close() = 0;

    /**
     * @brief 데이터 소스 타입 가져오기
     * @return 데이터 소스 타입
     */
    virtual DataSourceType getType() const = 0;

    /**
     * @brief 소스 정보 가져오기
     * @return 소스 정보 문자열
     */
    virtual std::string getSourceInfo() const = 0;

    /**
     * @brief 전체 프레임 수 가져오기 (비디오용)
     * @return 프레임 수
     */
    virtual int getTotalFrames() const = 0;

    /**
     * @brief 현재 프레임 인덱스 가져오기
     * @return 현재 프레임 인덱스
     */
    virtual int getCurrentFrameIndex() const = 0;

    /**
     * @brief 프레임 레이트 가져오기
     * @return FPS
     */
    virtual double getFrameRate() const = 0;

    /**
     * @brief 해상도 가져오기
     * @param width 너비 출력 포인터
     * @param height 높이 출력 포인터
     */
    virtual void getResolution(int* width, int* height) const = 0;

    /**
     * @brief 재생/일시정지 제어 (비디오용)
     * @param play 재생 여부
     */
    virtual void setPlayState(bool play) = 0;

    /**
     * @brief 재생 상태 가져오기
     * @return 재생 중 여부
     */
    virtual bool isPlaying() const = 0;

    /**
     * @brief 루프 재생 설정 (비디오용)
     * @param loop 루프 여부
     */
    virtual void setLooping(bool loop) = 0;

    /**
     * @brief 루프 재생 여부 확인
     * @return 루프 여부
     */
    virtual bool isLooping() const = 0;

    /**
     * @brief 에러 메시지 가져오기
     * @return 에러 메시지
     */
    virtual std::string getLastError() const = 0;

    /**
     * @brief 초기화 상태 확인
     * @return 초기화 완료 여부
     */
    virtual bool isInitialized() const = 0;
};
