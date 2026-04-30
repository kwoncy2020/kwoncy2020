#pragma once

#include "../IProcessingAlgorithm.h"
#include <opencv2/opencv.hpp>

/**
 * @brief 이미지 세그멘테이션 알고리즘
 * 
 * 이 클래스는 이미지를 여러 영역으로 분할하는
 * 세그멘테이션 알고리즘을 구현합니다.
 * 
 * 주요 기능:
 * - 다양한 세그멘테이션 방법 지원
 * - 자동 임계값 계산 (Otsu, Adaptive 등)
 * - 노이즈 제거 및 전처리
 * - 다중 클래스 세그멘테이션
 */
class SegmentationAlgorithm : public IProcessingAlgorithm {
public:
    /**
     * @brief 세그멘테이션 방법 열거형
     */
    enum class SegmentationMethod {
        Thresholding,        // 임계값 기반
        Otsu,              // Otsu 자동 임계값
        Adaptive,          // 적응적 임계값
        Watershed,          // 워터셰드
        KMeans,             // K-평균 클러스터링
        GrabCut,           // 그랩컷
        RegionGrowing       // 영역 확장
    };

    /**
     * @brief 세그멘테이션 클래스 정보
     */
    struct SegmentClass {
        int id;                    // 클래스 ID
        cv::Scalar color;           // 클래스 색상
        std::string name;          // 클래스 이름
        int pixelCount;            // 픽셀 수
        double percentage;          // 전체 이미지에서 비율
        
        SegmentClass() : id(0), pixelCount(0), percentage(0.0) {}
    };

private:
    SegmentationMethod m_method;                    // 현재 세그멘테이션 방법
    std::vector<SegmentClass> m_classes;               // 세그멘테이션 클래스들
    int m_numClasses;                                   // 클래스 수 (K-평균용)
    double m_thresholdValue;                             // 임계값
    int m_adaptiveMethod;                               // 적응적 임계값 방법
    int m_adaptiveBlockSize;                            // 적응적 임계값 블록 크기
    double m_adaptiveC;                                 // 적응적 임계값 상수
    int m_kmeansMaxIterations;                           // K-평균 최대 반복 횟수
    double m_kmeansEpsilon;                              // K-평균 수렴 기준
    bool m_useMorphology;                                // 형태학적 연산 사용 여부
    int m_morphKernelSize;                               // 형태학적 커널 크기

public:
    /**
     * @brief 생성자
     */
    SegmentationAlgorithm();
    
    /**
     * @brief 소멸자
     */
    ~SegmentationAlgorithm() override = default;

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

    // 세그멘테이션 특화 메서드
    void setSegmentationMethod(SegmentationMethod method);
    SegmentationMethod getSegmentationMethod() const;
    
    void setClasses(const std::vector<SegmentClass>& classes);
    std::vector<SegmentClass> getClasses() const;
    
    void addClass(const SegmentClass& segmentClass);
    void removeClass(int classId);
    void clearClasses();
    
    void setThresholdValue(double threshold);
    double getThresholdValue() const;
    
    void setAdaptiveMethod(int method, int blockSize, double c);
    void getAdaptiveParameters(int* method, int* blockSize, double* c) const;
    
    void setKMeansParameters(int numClasses, int maxIterations, double epsilon);
    void getKMeansParameters(int* numClasses, int* maxIterations, double* epsilon) const;
    
    void setMorphologyParameters(bool useMorphology, int kernelSize);
    void getMorphologyParameters(bool* useMorphology, int* kernelSize) const;
    
    // 클래스 관리 유틸리티
    void createDefaultClasses(int numClasses = 2);
    void createRandomClasses(int numClasses);
    SegmentClass getClass(int classId) const;

private:
    /**
     * @brief 임계값 기반 세그멘테이션
     * @param image 입력 이미지
     * @return 세그멘테이션 마스크
     */
    cv::Mat segmentByThresholding(const cv::Mat& image);
    
    /**
     * @brief Otsu 자동 임계값 세그멘테이션
     * @param image 입력 이미지
     * @return 세그멘테이션 마스크
     */
    cv::Mat segmentByOtsu(const cv::Mat& image);
    
    /**
     * @brief 적응적 임계값 세그멘테이션
     * @param image 입력 이미지
     * @return 세그멘테이션 마스크
     */
    cv::Mat segmentByAdaptive(const cv::Mat& image);
    
    /**
     * @brief 워터셰드 세그멘테이션
     * @param image 입력 이미지
     * @return 세그멘테이션 마스크
     */
    cv::Mat segmentByWatershed(const cv::Mat& image);
    
    /**
     * @brief K-평균 클러스터링 세그멘테이션
     * @param image 입력 이미지
     * @return 세그멘테이션 마스크
     */
    cv::Mat segmentByKMeans(const cv::Mat& image);
    
    /**
     * @brief 그랩컷 세그멘테이션
     * @param image 입력 이미지
     * @return 세그멘테이션 마스크
     */
    cv::Mat segmentByGrabCut(const cv::Mat& image);
    
    /**
     * @brief 영역 확장 세그멘테이션
     * @param image 입력 이미지
     * @return 세그멘테이션 마스크
     */
    cv::Mat segmentByRegionGrowing(const cv::Mat& image);
    
    /**
     * @brief 형태학적 연산 적용
     * @param mask 입력 마스크
     * @param operation 연산 타입
     * @param kernelSize 커널 크기
     * @return 처리된 마스크
     */
    cv::Mat applyMorphology(const cv::Mat& mask, int operation, int kernelSize);
    
    /**
     * @brief 클래스별 컬러 마스크 생성
     * @param mask 레이블 마스크
     * @return 컬러 마스크
     */
    cv::Mat createColorMask(const cv::Mat& mask);
    
    /**
     * @brief 세그멘테이션 결과 분석
     * @param mask 세그멘테이션 마스크
     */
    void analyzeSegments(const cv::Mat& mask);
    
    /**
     * @brief 이미지 전처리
     * @param image 입력 이미지
     * @return 전처리된 이미지
     */
    cv::Mat preprocessImage(const cv::Mat& image);
    
    /**
     * @brief 색상 공간 변환
     * @param image 입력 이미지
     * @param space 변환할 색상 공간
     * @return 변환된 이미지
     */
    cv::Mat convertColorSpace(const cv::Mat& image, int space);
    
    /**
     * @brief 시드 포인트 생성 (워터셰드용)
     * @param image 입력 이미지
     * @return 시드 포인트 목록
     */
    std::vector<cv::Point> generateSeeds(const cv::Mat& image);
};
