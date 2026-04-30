#pragma once

#include "../IProcessingAlgorithm.h"
#include <opencv2/opencv.hpp>

/**
 * @brief 바운딩박스 감지 알고리즘
 * 
 * 이 클래스는 이미지에서 객체의 바운딩박스를 감지하는
 * 알고리즘을 구현합니다.
 * 
 * 주요 기능:
 * - 다양한 객체 감지 방법 지원
 * - 신뢰도 임계값 조절
 * - 멀티스케일 객체 감지
 * - NMS (Non-Maximum Suppression) 적용
 */
class BoundingBoxDetection : public IProcessingAlgorithm {
public:
    /**
     * @brief 감지 방법 열거형
     */
    enum class DetectionMethod {
        TemplateMatching,     // 템플릿 매칭
        ContourDetection,    // 컨투어 감지
        EdgeDetection,        // 에지 감지
        ColorSegmentation     // 색상 분할
    };

private:
    DetectionMethod m_method;                    // 현재 감지 방법
    double m_confidenceThreshold;                  // 신뢰도 임계값
    double m_nmsThreshold;                         // NMS 임계값
    cv::Size m_minSize;                           // 최소 객체 크기
    cv::Size m_maxSize;                           // 최대 객체 크기
    cv::Mat m_template;                           // 템플릿 이미지 (템플릿 매칭용)
    std::vector<cv::Scalar> m_targetColors;       // 목표 색상 (색상 분할용)
    bool m_useMultiScale;                          // 멀티스케일 사용 여부
    double m_scaleFactor;                           // 스케일 팩터
    int m_minNeighbors;                             // 최소 이웃 수

public:
    /**
     * @brief 생성자
     */
    BoundingBoxDetection();
    
    /**
     * @brief 소멸자
     */
    ~BoundingBoxDetection() override = default;

    // IProcessingAlgorithm 인터페이스 구현
    std::string getName() const override;
    std::string getDescription() const override;
    ProcessingResult process(const cv::Mat& input) override;
    bool supportsAsyncProcessing() const override;
    void setParameter(const std::string& name, double value) override;
    double getParameter(const std::string& name) const override;
    std::vector<std::string> getSupportedParameters() const override;
    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override;
    bool canProcess(int imageType) const override;
    std::string getAlgorithmType() const override;

    // 바운딩박스 감지 특화 메서드
    void setDetectionMethod(DetectionMethod method);
    DetectionMethod getDetectionMethod() const;
    
    void setTemplateImage(const cv::Mat& templateImage);
    cv::Mat getTemplateImage() const;
    
    void setTargetColors(const std::vector<cv::Scalar>& colors);
    std::vector<cv::Scalar> getTargetColors() const;
    
    void setMinObjectSize(const cv::Size& size);
    cv::Size getMinObjectSize() const;
    
    void setMaxObjectSize(const cv::Size& size);
    cv::Size getMaxObjectSize() const;
    
    void setUseMultiScale(bool useMultiScale);
    bool getUseMultiScale() const;

private:
    /**
     * @brief 템플릿 매칭으로 바운딩박스 감지
     * @param image 입력 이미지
     * @return 감지된 바운딩박스 목록
     */
    std::vector<cv::Rect> detectByTemplateMatching(const cv::Mat& image);
    
    /**
     * @brief 컨투어 감지로 바운딩박스 감지
     * @param image 입력 이미지
     * @return 감지된 바운딩박스 목록
     */
    std::vector<cv::Rect> detectByContours(const cv::Mat& image);
    
    /**
     * @brief 에지 감지로 바운딩박스 감지
     * @param image 입력 이미지
     * @return 감지된 바운딩박스 목록
     */
    std::vector<cv::Rect> detectByEdges(const cv::Mat& image);
    
    /**
     * @brief 색상 분할로 바운딩박스 감지
     * @param image 입력 이미지
     * @return 감지된 바운딩박스 목록
     */
    std::vector<cv::Rect> detectByColorSegmentation(const cv::Mat& image);
    
    /**
     * @brief NMS (Non-Maximum Suppression) 적용
     * @param boxes 바운딩박스 목록
     * @param scores 신뢰도 점수 목록
     * @param threshold NMS 임계값
     * @return NMS 적용 후 바운딩박스 인덱스
     */
    std::vector<int> applyNMS(const std::vector<cv::Rect>& boxes, 
                             const std::vector<double>& scores, 
                             double threshold);
    
    /**
     * @brief 바운딩박스 필터링
     * @param boxes 바운딩박스 목록
     * @return 필터링된 바운딩박스 목록
     */
    std::vector<cv::Rect> filterBoundingBoxes(const std::vector<cv::Rect>& boxes);
    
    /**
     * @brief 이미지 전처리
     * @param image 입력 이미지
     * @return 전처리된 이미지
     */
    cv::Mat preprocessImage(const cv::Mat& image);
    
    /**
     * @brief 전처리 파라미터 설정
     */
    void setupPreprocessingParameters();
};
