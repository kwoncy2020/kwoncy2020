#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

/**
 * @brief 처리 결과 구조체
 */
struct ProcessingResult {
    cv::Mat processedImage;           // 처리된 이미지
    std::vector<cv::Rect> boundingBoxes; // 바운딩박스 목록
    cv::Mat segmentationMask;        // 세그멘테이션 마스크
    std::vector<cv::Point> keypoints; // 키포인트 목록
    std::string algorithmName;        // 알고리즘 이름
    double processingTime;            // 처리 시간 (ms)
    bool success;                   // 성공 여부
    std::string errorMessage;         // 에러 메시지
    
    ProcessingResult() 
        : processingTime(0.0), success(false) {}
};

/**
 * @brief 처리 작업 구조체
 */
struct ProcessingTask {
    cv::Mat inputImage;              // 입력 이미지
    std::string algorithmName;        // 알고리즘 이름
    std::map<std::string, double> parameters; // 알고리즘 파라미터
    std::function<void(const ProcessingResult&)> callback; // 완료 콜백
    int priority;                    // 작업 우선순위
    
    ProcessingTask() : priority(0) {}
};

/**
 * @brief 영상처리 알고리즘 인터페이스
 * 
 * 이 인터페이스는 다양한 영상처리 알고리즘을
 * 통합된 방식으로 처리하기 위한 공통 API를 제공합니다.
 * 
 * 주요 기능:
 * - 이미지 처리
 * - 파라미터 설정
 * - 비동기 처리 지원
 * - 결과 포맷 표준화
 */
class IProcessingAlgorithm {
public:
    virtual ~IProcessingAlgorithm() = default;

    /**
     * @brief 알고리즘 이름 가져오기
     * @return 알고리즘 이름
     */
    virtual std::string getName() const = 0;

    /**
     * @brief 알고리즘 설명 가져오기
     * @return 알고리즘 설명
     */
    virtual std::string getDescription() const = 0;

    /**
     * @brief 알고리즘 처리
     * @param input 입력 이미지
     * @return 처리 결과
     */
    virtual ProcessingResult process(const cv::Mat& input) = 0;

    /**
     * @brief 비동기 처리 지원 여부
     * @return 비동기 처리 가능 여부
     */
    virtual bool supportsAsyncProcessing() const = 0;

    /**
     * @brief 파라미터 설정
     * @param name 파라미터 이름
     * @param value 파라미터 값
     */
    virtual void setParameter(const std::string& name, double value) = 0;

    /**
     * @brief 파라미터 값 가져오기
     * @param name 파라미터 이름
     * @return 파라미터 값
     */
    virtual double getParameter(const std::string& name) const = 0;

    /**
     * @brief 지원하는 파라미터 목록 가져오기
     * @return 파라미터 이름 목록
     */
    virtual std::vector<std::string> getSupportedParameters() const = 0;

    /**
     * @brief 알고리즘 초기화
     * @return 성공 여부
     */
    virtual bool initialize() = 0;

    /**
     * @brief 알고리즘 종료
     */
    virtual void cleanup() = 0;

    /**
     * @brief 초기화 상태 확인
     * @return 초기화 완료 여부
     */
    virtual bool isInitialized() const = 0;

    /**
     * @brief 처리 가능한 이미지 타입 확인
     * @param imageType OpenCV 이미지 타입
     * @return 처리 가능 여부
     */
    virtual bool canProcess(int imageType) const = 0;

    /**
     * @brief 알고리즘 타입 확인
     * @return 알고리즘 타입 문자열
     */
    virtual std::string getAlgorithmType() const = 0;

protected:
    /**
     * @brief 파라미터 맵
     */
    std::map<std::string, double> m_parameters;
    
    /**
     * @brief 초기화 상태
     */
    bool m_initialized = false;
    
    /**
     * @brief 파라미터 값 가져오기 (내부용)
     * @param name 파라미터 이름
     * @param defaultValue 기본값
     * @return 파라미터 값
     */
    double getParameterSafe(const std::string& name, double defaultValue = 0.0) const {
        auto it = m_parameters.find(name);
        return (it != m_parameters.end()) ? it->second : defaultValue;
    }
};
