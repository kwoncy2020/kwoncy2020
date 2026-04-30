#include "processing/algorithms/SegmentationAlgorithm.h"
#include <iostream>
#include <random>
#include <algorithm>

SegmentationAlgorithm::SegmentationAlgorithm() 
    : m_method(SegmentationMethod::Otsu),
      m_thresholdValue(128.0),
      m_adaptiveMethod(cv::ADAPTIVE_THRESH_MEAN_C),
      m_adaptiveBlockSize(11),
      m_adaptiveC(5.0),
      m_kmeansMaxIterations(10),
      m_kmeansEpsilon(1.0),
      m_useMorphology(true),
      m_morphKernelSize(3),
      m_numClasses(2) {
    createDefaultClasses();
}

std::string SegmentationAlgorithm::getName() const {
    return "Image Segmentation";
}

std::string SegmentationAlgorithm::getDescription() const {
    return "Segments image into different regions using various methods";
}

ProcessingResult SegmentationAlgorithm::process(const cv::Mat& input) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    ProcessingResult result;
    result.algorithmName = getName();
    result.success = false;
    
    if (input.empty()) {
        result.errorMessage = "Input image is empty";
        return result;
    }
    
    try {
        // 이미지 전처리
        cv::Mat processedImage = preprocessImage(input);
        
        // 세그멘테이션 방법에 따른 처리
        cv::Mat mask;
        
        switch (m_method) {
            case SegmentationMethod::Thresholding:
                mask = segmentByThresholding(processedImage);
                break;
                
            case SegmentationMethod::Otsu:
                mask = segmentByOtsu(processedImage);
                break;
                
            case SegmentationMethod::Adaptive:
                mask = segmentByAdaptive(processedImage);
                break;
                
            case SegmentationMethod::Watershed:
                mask = segmentByWatershed(processedImage);
                break;
                
            case SegmentationMethod::KMeans:
                mask = segmentByKMeans(processedImage);
                break;
                
            case SegmentationMethod::GrabCut:
                mask = segmentByGrabCut(processedImage);
                break;
                
            case SegmentationMethod::RegionGrowing:
                mask = segmentByRegionGrowing(processedImage);
                break;
                
            default:
                result.errorMessage = "Unknown segmentation method";
                return result;
        }
        
        // 세그멘테이션 결과 분석
        analyzeSegments(mask);
        
        // 결과 설정
        result.segmentationMask = mask.clone();
        result.processedImage = createColorMask(mask);
        result.success = true;
        
        std::cout << "Segmented image into " << m_classes.size() << " classes" << std::endl;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Segmentation error: " + std::string(e.what());
        std::cerr << "Segmentation error: " << e.what() << std::endl;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    return result;
}

bool SegmentationAlgorithm::supportsAsyncProcessing() const {
    return true;
}

void SegmentationAlgorithm::setParameter(const std::string& name, double value) {
    m_parameters[name] = value;
    
    // 내부 변수 업데이트
    if (name == "threshold_value") {
        m_thresholdValue = value;
    } else if (name == "adaptive_method") {
        m_adaptiveMethod = static_cast<int>(value);
    } else if (name == "adaptive_block_size") {
        m_adaptiveBlockSize = static_cast<int>(value);
    } else if (name == "adaptive_c") {
        m_adaptiveC = value;
    } else if (name == "num_classes") {
        m_numClasses = static_cast<int>(value);
        createDefaultClasses(m_numClasses);
    } else if (name == "kmeans_max_iterations") {
        m_kmeansMaxIterations = static_cast<int>(value);
    } else if (name == "kmeans_epsilon") {
        m_kmeansEpsilon = value;
    } else if (name == "use_morphology") {
        m_useMorphology = (value != 0.0);
    } else if (name == "morph_kernel_size") {
        m_morphKernelSize = static_cast<int>(value);
    }
}

double SegmentationAlgorithm::getParameter(const std::string& name) const {
    return getParameterSafe(name);
}

std::vector<std::string> SegmentationAlgorithm::getSupportedParameters() const {
    return {
        "threshold_value",
        "adaptive_method",
        "adaptive_block_size", 
        "adaptive_c",
        "num_classes",
        "kmeans_max_iterations",
        "kmeans_epsilon",
        "use_morphology",
        "morph_kernel_size"
    };
}

bool SegmentationAlgorithm::initialize() {
    m_initialized = true;
    return true;
}

void SegmentationAlgorithm::cleanup() {
    clearClasses();
    m_initialized = false;
}

bool SegmentationAlgorithm::isInitialized() const {
    return m_initialized;
}

bool SegmentationAlgorithm::canProcess(int imageType) const {
    // 그레이스케일 또는 컬러 이미지 지원
    return (imageType == CV_8UC1) || (imageType == CV_8UC3);
}

std::string SegmentationAlgorithm::getAlgorithmType() const {
    return "Image Segmentation";
}

// 특화 메서드

void SegmentationAlgorithm::setSegmentationMethod(SegmentationMethod method) {
    m_method = method;
}

SegmentationAlgorithm::SegmentationMethod SegmentationAlgorithm::getSegmentationMethod() const {
    return m_method;
}

void SegmentationAlgorithm::setClasses(const std::vector<SegmentClass>& classes) {
    m_classes = classes;
    m_numClasses = static_cast<int>(classes.size());
}

std::vector<SegmentationAlgorithm::SegmentClass> SegmentationAlgorithm::getClasses() const {
    return m_classes;
}

void SegmentationAlgorithm::addClass(const SegmentClass& segmentClass) {
    m_classes.push_back(segmentClass);
    m_numClasses = static_cast<int>(m_classes.size());
}

void SegmentationAlgorithm::removeClass(int classId) {
    m_classes.erase(
        std::remove_if(m_classes.begin(), m_classes.end(),
            [classId](const SegmentClass& cls) { return cls.id == classId; }),
        m_classes.end()
    );
    m_numClasses = static_cast<int>(m_classes.size());
}

void SegmentationAlgorithm::clearClasses() {
    m_classes.clear();
    m_numClasses = 0;
}

void SegmentationAlgorithm::setThresholdValue(double threshold) {
    m_thresholdValue = threshold;
}

double SegmentationAlgorithm::getThresholdValue() const {
    return m_thresholdValue;
}

void SegmentationAlgorithm::setAdaptiveMethod(int method, int blockSize, double c) {
    m_adaptiveMethod = method;
    m_adaptiveBlockSize = blockSize;
    m_adaptiveC = c;
}

void SegmentationAlgorithm::getAdaptiveParameters(int* method, int* blockSize, double* c) const {
    if (method) *method = m_adaptiveMethod;
    if (blockSize) *blockSize = m_adaptiveBlockSize;
    if (c) *c = m_adaptiveC;
}

void SegmentationAlgorithm::setKMeansParameters(int numClasses, int maxIterations, double epsilon) {
    m_numClasses = numClasses;
    m_kmeansMaxIterations = maxIterations;
    m_kmeansEpsilon = epsilon;
    createDefaultClasses(numClasses);
}

void SegmentationAlgorithm::getKMeansParameters(int* numClasses, int* maxIterations, double* epsilon) const {
    if (numClasses) *numClasses = m_numClasses;
    if (maxIterations) *maxIterations = m_kmeansMaxIterations;
    if (epsilon) *epsilon = m_kmeansEpsilon;
}

void SegmentationAlgorithm::setMorphologyParameters(bool useMorphology, int kernelSize) {
    m_useMorphology = useMorphology;
    m_morphKernelSize = kernelSize;
}

void SegmentationAlgorithm::getMorphologyParameters(bool* useMorphology, int* kernelSize) const {
    if (useMorphology) *useMorphology = m_useMorphology;
    if (kernelSize) *kernelSize = m_morphKernelSize;
}

// Private 메서드

cv::Mat SegmentationAlgorithm::segmentByThresholding(const cv::Mat& image) {
    cv::Mat mask;
    cv::threshold(image, mask, m_thresholdValue, 255, cv::THRESH_BINARY);
    return mask;
}

cv::Mat SegmentationAlgorithm::segmentByOtsu(const cv::Mat& image) {
    cv::Mat mask;
    cv::threshold(image, mask, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
    return mask;
}

cv::Mat SegmentationAlgorithm::segmentByAdaptive(const cv::Mat& image) {
    cv::Mat mask;
    cv::adaptiveThreshold(image, mask, 255, m_adaptiveMethod, 
                         cv::THRESH_BINARY, m_adaptiveBlockSize, m_adaptiveC);
    return mask;
}

cv::Mat SegmentationAlgorithm::segmentByWatershed(const cv::Mat& image) {
    cv::Mat gray = image;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    }
    
    // 마커 생성
    cv::Mat markers = cv::Mat::zeros(gray.size(), CV_32SC1);
    
    // 전경/배경 마커 생성
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    
    // 전경 마커 설정
    cv::Mat foreground;
    cv::distanceTransform(binary, foreground, cv::DIST_L2, 5);
    cv::normalize(foreground, foreground, 0, 1, cv::NORM_MINMAX);
    
    cv::Mat sure_fg;
    cv::threshold(foreground, sure_fg, 0.7, 1.0, cv::THRESH_BINARY);
    sure_fg.convertTo(sure_fg, CV_8U);
    
    // 배경 마커 설정
    cv::Mat background;
    cv::dilate(binary, background, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)), 2);
    
    cv::Mat sure_bg;
    cv::threshold(background, sure_bg, 0.01, 1.0, cv::THRESH_BINARY);
    sure_bg.convertTo(sure_bg, CV_8U);
    
    // 마커 결합
    for (int i = 0; i < markers.rows; i++) {
        for (int j = 0; j < markers.cols; j++) {
            if (sure_bg.at<uchar>(i, j) == 255) {
                markers.at<int>(i, j) = 1; // 배경
            } else if (sure_fg.at<uchar>(i, j) == 255) {
                markers.at<int>(i, j) = 2; // 전경
            } else {
                markers.at<int>(i, j) = 0; // 미지정
            }
        }
    }
    
    // 워터셰드 실행
    cv::watershed(image, markers);
    
    // 결과 마스크 생성
    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);
    for (int i = 0; i < markers.rows; i++) {
        for (int j = 0; j < markers.cols; j++) {
            if (markers.at<int>(i, j) == 2) {
                mask.at<uchar>(i, j) = 255;
            }
        }
    }
    
    return mask;
}

cv::Mat SegmentationAlgorithm::segmentByKMeans(const cv::Mat& image) {
    cv::Mat data;
    image.reshape(1, image.rows * image.cols).convertTo(data, CV_32F);
    
    // K-평균 클러스터링
    cv::Mat labels, centers;
    cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, m_kmeansMaxIterations, m_kmeansEpsilon);
    cv::kmeans(data, m_numClasses, criteria, cv::KMEANS_PP_CENTERS, cv::KMEANS_RANDOM_CENTERS, 
               labels, centers);
    
    // 결과 마스크 생성
    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);
    for (int i = 0; i < labels.rows; i++) {
        int label = labels.at<int>(i);
        mask.at<uchar>(i / image.cols, i % image.cols) = static_cast<uchar>(label * (255 / (m_numClasses - 1)));
    }
    
    return mask;
}

cv::Mat SegmentationAlgorithm::segmentByGrabCut(const cv::Mat& image) {
    // 그랩컷은 복잡하여 간단한 구현
    // 실제 구현에서는 OpenCV의 cv::grabCut 함수 사용 필요
    
    // 간단한 전경/배경 분할로 대체
    cv::Mat mask;
    cv::threshold(image, mask, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
    
    // 형태학적 연산으로 정제
    if (m_useMorphology) {
        mask = applyMorphology(mask, cv::MORPH_OPEN, m_morphKernelSize);
        mask = applyMorphology(mask, cv::MORPH_CLOSE, m_morphKernelSize);
    }
    
    return mask;
}

cv::Mat SegmentationAlgorithm::segmentByRegionGrowing(const cv::Mat& image) {
    // 영역 확장은 복잡하여 간단한 구현
    cv::Mat mask;
    cv::threshold(image, mask, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
    
    // 시드 포인트 생성 (중앙에서 시작)
    std::vector<cv::Point> seeds = generateSeeds(image);
    
    // 각 시드에서 영역 확장
    for (const cv::Point& seed : seeds) {
        cv::floodFill(mask, seed, cv::Scalar(255), 0, cv::FLOODFILL_FIXED_RANGE);
    }
    
    return mask;
}

cv::Mat SegmentationAlgorithm::applyMorphology(const cv::Mat& mask, int operation, int kernelSize) {
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
    cv::Mat result;
    cv::morphologyEx(mask, result, operation, kernel);
    return result;
}

cv::Mat SegmentationAlgorithm::createColorMask(const cv::Mat& mask) {
    cv::Mat colorMask = cv::Mat::zeros(mask.size(), CV_8UC3);
    
    // 각 클래스에 따른 색상 할당
    for (size_t i = 0; i < m_classes.size(); ++i) {
        if (i < m_classes.size()) {
            cv::Scalar color = m_classes[i].color;
            cv::Mat classMask = (mask == static_cast<uchar>(i));
            colorMask.setTo(color, classMask);
        }
    }
    
    return colorMask;
}

void SegmentationAlgorithm::analyzeSegments(const cv::Mat& mask) {
    // 각 클래스의 픽셀 수와 비율 계산
    int totalPixels = mask.rows * mask.cols;
    
    for (size_t i = 0; i < m_classes.size(); ++i) {
        if (i < m_classes.size()) {
            cv::Mat classMask = (mask == static_cast<uchar>(i));
            m_classes[i].pixelCount = cv::countNonZero(classMask);
            m_classes[i].percentage = static_cast<double>(m_classes[i].pixelCount) / totalPixels * 100.0;
        }
    }
}

cv::Mat SegmentationAlgorithm::preprocessImage(const cv::Mat& image) {
    cv::Mat processed;
    
    // 가우시안 블러로 노이즈 제거
    cv::GaussianBlur(image, processed, cv::Size(3, 3), 0);
    
    return processed;
}

cv::Mat SegmentationAlgorithm::convertColorSpace(const cv::Mat& image, int space) {
    cv::Mat converted;
    
    switch (space) {
        case 0: // HSV
            cv::cvtColor(image, converted, cv::COLOR_BGR2HSV);
            break;
        case 1: // Lab
            cv::cvtColor(image, converted, cv::COLOR_BGR2Lab);
            break;
        case 2: // Gray
            if (image.channels() == 3) {
                cv::cvtColor(image, converted, cv::COLOR_BGR2GRAY);
            } else {
                converted = image.clone();
            }
            break;
        default:
            converted = image.clone();
            break;
    }
    
    return converted;
}

std::vector<cv::Point> SegmentationAlgorithm::generateSeeds(const cv::Mat& image) {
    std::vector<cv::Point> seeds;
    
    // 이미지 중앙에서 시드 생성
    cv::Point center(image.cols / 2, image.rows / 2);
    seeds.push_back(center);
    
    // 그리드 기반 시드 생성
    int gridSize = 50;
    for (int y = gridSize; y < image.rows; y += gridSize) {
        for (int x = gridSize; x < image.cols; x += gridSize) {
            seeds.push_back(cv::Point(x, y));
        }
    }
    
    return seeds;
}

void SegmentationAlgorithm::createDefaultClasses(int numClasses) {
    clearClasses();
    
    // 기본 클래스 생성
    for (int i = 0; i < numClasses; ++i) {
        SegmentClass cls;
        cls.id = i;
        cls.name = "Class " + std::to_string(i + 1);
        cls.pixelCount = 0;
        cls.percentage = 0.0;
        
        // 기본 색상 할당
        switch (i % 6) {
            case 0: cls.color = cv::Scalar(255, 0, 0); break;     // 빨강
            case 1: cls.color = cv::Scalar(0, 255, 0); break;     // 초록
            case 2: cls.color = cv::Scalar(0, 0, 255); break;     // 파랑
            case 3: cls.color = cv::Scalar(255, 255, 0); break; // 노랑
            case 4: cls.color = cv::Scalar(255, 0, 255); break; // 마젠타
            case 5: cls.color = cv::Scalar(0, 255, 255); break; // 시안
        }
        
        m_classes.push_back(cls);
    }
}

void SegmentationAlgorithm::createRandomClasses(int numClasses) {
    clearClasses();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (int i = 0; i < numClasses; ++i) {
        SegmentClass cls;
        cls.id = i;
        cls.name = "Class " + std::to_string(i + 1);
        cls.pixelCount = 0;
        cls.percentage = 0.0;
        cls.color = cv::Scalar(dis(gen), dis(gen), dis(gen));
        
        m_classes.push_back(cls);
    }
}

SegmentClass SegmentationAlgorithm::getClass(int classId) const {
    for (const auto& cls : m_classes) {
        if (cls.id == classId) {
            return cls;
        }
    }
    
    SegmentClass notFound;
    notFound.id = -1;
    return notFound;
}
