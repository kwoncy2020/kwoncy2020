#pragma once

#include "../IProcessingAlgorithm.h"
#include <opencv2/opencv.hpp>

/**
 * @brief 이미지 매칭 알고리즘
 * 
 * 이 클래스는 두 이미지 간의 유사도를 계산하거나
 * 템플릿 이미지를 대상 이미지에서 찾는 알고리즘을 구현합니다.
 * 
 * 주요 기능:
 * - 다양한 매칭 방법 지원 (템플릿 매칭, 특징점 매칭)
 * - 유사도 계산 및 점수화
 * - 다중 템플릿 지원
 * - 회전 및 스케일 불변 매칭
 */
class MatchingAlgorithm : public IProcessingAlgorithm {
public:
    /**
     * @brief 매칭 방법 열거형
     */
    enum class MatchingMethod {
        TemplateMatching,     // 템플릿 매칭
        FeatureMatching,     // 특징점 매칭
        HistogramMatching,   // 히스토그램 매칭
        StructuralSimilarity // 구조적 유사도 (SSIM)
    };

    /**
     * @brief 매칭 결과 구조체
     */
    struct MatchResult {
        cv::Point location;           // 매칭 위치
        double confidence;            // 신뢰도 (0.0-1.0)
        double similarity;            // 유사도 점수
        cv::Rect boundingBox;         // 바운딩박스
        cv::Mat matchedRegion;        // 매칭된 영역
        std::vector<cv::KeyPoint> keypoints; // 특징점들
        std::vector<cv::DMatch> matches;     // 매치들
        
        MatchResult() : confidence(0.0), similarity(0.0) {}
    };

private:
    MatchingMethod m_method;                    // 현재 매칭 방법
    cv::Mat m_template;                           // 템플릿 이미지
    std::vector<cv::Mat> m_templates;             // 다중 템플릿 이미지들
    double m_matchThreshold;                      // 매칭 임계값
    int m_templateMatchingMethod;                 // 템플릿 매칭 방법
    std::string m_featureDetectorType;            // 특징점 검출기 타입
    std::string m_featureMatcherType;            // 특징점 매처 타입
    bool m_useMultiScale;                         // 멀티스케일 사용 여부
    double m_scaleRangeMin;                       // 스케일 범위 최소값
    double m_scaleRangeMax;                       // 스케일 범위 최대값
    double m_scaleStep;                           // 스케일 스텝
    bool m_useRotationInvariant;                   // 회전 불변성 사용 여부
    std::vector<MatchResult> m_matchResults;      // 매칭 결과들

public:
    /**
     * @brief 생성자
     */
    MatchingAlgorithm();
    
    /**
     * @brief 소멸자
     */
    ~MatchingAlgorithm() override = default;

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

    // 매칭 알고리즘 특화 메서드
    void setMatchingMethod(MatchingMethod method);
    MatchingMethod getMatchingMethod() const;
    
    void setTemplate(const cv::Mat& templateImage);
    cv::Mat getTemplate() const;
    
    void addTemplate(const cv::Mat& templateImage);
    void clearTemplates();
    std::vector<cv::Mat> getTemplates() const;
    
    void setMatchThreshold(double threshold);
    double getMatchThreshold() const;
    
    void setTemplateMatchingMethod(int method);
    int getTemplateMatchingMethod() const;
    
    void setFeatureDetector(const std::string& detectorType);
    void setFeatureMatcher(const std::string& matcherType);
    std::string getFeatureDetectorType() const;
    std::string getFeatureMatcherType() const;
    
    void setMultiScaleParameters(bool useMultiScale, double minScale, double maxScale, double step);
    void getMultiScaleParameters(bool* useMultiScale, double* minScale, double* maxScale, double* step) const;
    
    void setRotationInvariant(bool useRotationInvariant);
    bool getRotationInvariant() const;
    
    std::vector<MatchResult> getMatchResults() const;
    MatchResult getBestMatch() const;

private:
    /**
     * @brief 템플릿 매칭 수행
     * @param image 입력 이미지
     * @param templateImage 템플릿 이미지
     * @return 매칭 결과
     */
    MatchResult performTemplateMatching(const cv::Mat& image, const cv::Mat& templateImage);
    
    /**
     * @brief 특징점 매칭 수행
     * @param image 입력 이미지
     * @param templateImage 템플릿 이미지
     * @return 매칭 결과
     */
    MatchResult performFeatureMatching(const cv::Mat& image, const cv::Mat& templateImage);
    
    /**
     * @brief 히스토그램 매칭 수행
     * @param image 입력 이미지
     * @param templateImage 템플릿 이미지
     * @return 매칭 결과
     */
    MatchResult performHistogramMatching(const cv::Mat& image, const cv::Mat& templateImage);
    
    /**
     * @brief 구조적 유사도 계산 (SSIM)
     * @param image 입력 이미지
     * @param templateImage 템플릿 이미지
     * @return 유사도 점수
     */
    double calculateSSIM(const cv::Mat& image, const cv::Mat& templateImage);
    
    /**
     * @brief 멀티스케일 템플릿 매칭
     * @param image 입력 이미지
     * @param templateImage 템플릿 이미지
     * @return 최적 매칭 결과
     */
    MatchResult performMultiScaleMatching(const cv::Mat& image, const cv::Mat& templateImage);
    
    /**
     * @brief 회전 불변 템플릿 매칭
     * @param image 입력 이미지
     * @param templateImage 템플릿 이미지
     * @return 최적 매칭 결과
     */
    MatchResult performRotationInvariantMatching(const cv::Mat& image, const cv::Mat& templateImage);
    
    /**
     * @brief 특징점 검출 및 추출
     * @param image 입력 이미지
     * @param detectorType 검출기 타입
     * @return 특징점들
     */
    std::vector<cv::KeyPoint> extractFeatures(const cv::Mat& image, const std::string& detectorType);
    
    /**
     * @brief 특징점 기술자 계산
     * @param image 입력 이미지
     * @param keypoints 특징점들
     * @param detectorType 검출기 타입
     * @return 기술자
     */
    cv::Mat computeDescriptors(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, const std::string& detectorType);
    
    /**
     * @brief 특징점 매칭
     * @param descriptors1 첫 번째 이미지 기술자
     * @param descriptors2 두 번째 이미지 기술자
     * @param matcherType 매처 타입
     * @return 매치들
     */
    std::vector<cv::DMatch> matchFeatures(const cv::Mat& descriptors1, const cv::Mat& descriptors2, const std::string& matcherType);
    
    /**
     * @brief 매치 필터링 (RANSAC 등)
     * @param keypoints1 첫 번째 이미지 특징점들
     * @param keypoints2 두 번째 이미지 특징점들
     * @param matches 매치들
     * @return 필터링된 매치들
     */
    std::vector<cv::DMatch> filterMatches(const std::vector<cv::KeyPoint>& keypoints1,
                                        const std::vector<cv::KeyPoint>& keypoints2,
                                        const std::vector<cv::DMatch>& matches);
    
    /**
     * @brief 이미지 전처리
     * @param image 입력 이미지
     * @return 전처리된 이미지
     */
    cv::Mat preprocessImage(const cv::Mat& image);
    
    /**
     * @brief 히스토그램 계산
     * @param image 입력 이미지
     * @param bins 히스토그램 빈 수
     * @return 히스토그램
     */
    cv::Mat calculateHistogram(const cv::Mat& image, int bins = 256);
    
    /**
     * @brief 히스토그램 비교
     * @param hist1 첫 번째 히스토그램
     * @param hist2 두 번째 히스토그램
     * @param method 비교 방법
     * @return 비교 점수
     */
    double compareHistograms(const cv::Mat& hist1, const cv::Mat& hist2, int method);
    
    /**
     * @brief 매칭 결과 생성
     * @param location 매칭 위치
     * @param confidence 신뢰도
     * @param similarity 유사도
     * @param templateSize 템플릿 크기
     * @return 매칭 결과
     */
    MatchResult createMatchResult(const cv::Point& location, double confidence, double similarity, const cv::Size& templateSize);
};
