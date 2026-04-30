#include "processing/algorithms/BoundingBoxDetection.h"
#include <iostream>
#include <algorithm>

BoundingBoxDetection::BoundingBoxDetection() 
    : m_method(DetectionMethod::ContourDetection),
      m_confidenceThreshold(0.5),
      m_nmsThreshold(0.4),
      m_minSize(10, 10),
      m_maxSize(1000, 1000),
      m_useMultiScale(true),
      m_scaleFactor(1.1),
      m_minNeighbors(3) {
}

std::string BoundingBoxDetection::getName() const {
    return "Bounding Box Detection";
}

std::string BoundingBoxDetection::getDescription() const {
    return "Detects objects and generates bounding boxes using various methods";
}

ProcessingResult BoundingBoxDetection::process(const cv::Mat& input) {
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
        
        // 감지 방법에 따른 처리
        std::vector<cv::Rect> boxes;
        
        switch (m_method) {
            case DetectionMethod::TemplateMatching:
                boxes = detectByTemplateMatching(processedImage);
                break;
                
            case DetectionMethod::ContourDetection:
                boxes = detectByContours(processedImage);
                break;
                
            case DetectionMethod::EdgeDetection:
                boxes = detectByEdges(processedImage);
                break;
                
            case DetectionMethod::ColorSegmentation:
                boxes = detectByColorSegmentation(processedImage);
                break;
                
            default:
                result.errorMessage = "Unknown detection method";
                return result;
        }
        
        // 바운딩박스 필터링
        boxes = filterBoundingBoxes(boxes);
        
        // 결과 설정
        result.boundingBoxes = boxes;
        result.processedImage = processedImage.clone();
        result.success = true;
        
        std::cout << "Detected " << boxes.size() << " bounding boxes" << std::endl;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Processing error: " + std::string(e.what());
        std::cerr << "Bounding box detection error: " << e.what() << std::endl;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    return result;
}

bool BoundingBoxDetection::supportsAsyncProcessing() const {
    return true;
}

void BoundingBoxDetection::setParameter(const std::string& name, double value) {
    m_parameters[name] = value;
    
    // 내부 변수 업데이트
    if (name == "confidence_threshold") {
        m_confidenceThreshold = value;
    } else if (name == "nms_threshold") {
        m_nmsThreshold = value;
    } else if (name == "min_width") {
        m_minSize.width = static_cast<int>(value);
    } else if (name == "min_height") {
        m_minSize.height = static_cast<int>(value);
    } else if (name == "max_width") {
        m_maxSize.width = static_cast<int>(value);
    } else if (name == "max_height") {
        m_maxSize.height = static_cast<int>(value);
    } else if (name == "use_multiscale") {
        m_useMultiScale = (value != 0.0);
    } else if (name == "scale_factor") {
        m_scaleFactor = value;
    } else if (name == "min_neighbors") {
        m_minNeighbors = static_cast<int>(value);
    }
}

double BoundingBoxDetection::getParameter(const std::string& name) const {
    return getParameterSafe(name);
}

std::vector<std::string> BoundingBoxDetection::getSupportedParameters() const {
    return {
        "confidence_threshold",
        "nms_threshold", 
        "min_width",
        "min_height",
        "max_width",
        "max_height",
        "use_multiscale",
        "scale_factor",
        "min_neighbors"
    };
}

bool BoundingBoxDetection::initialize() {
    setupPreprocessingParameters();
    m_initialized = true;
    return true;
}

void BoundingBoxDetection::cleanup() {
    m_template.release();
    m_targetColors.clear();
    m_initialized = false;
}

bool BoundingBoxDetection::isInitialized() const {
    return m_initialized;
}

bool BoundingBoxDetection::canProcess(int imageType) const {
    // 그레이스케일 또는 컬러 이미지 지원
    return (imageType == CV_8UC1) || (imageType == CV_8UC3) || (imageType == CV_8UC4);
}

std::string BoundingBoxDetection::getAlgorithmType() const {
    return "Object Detection";
}

// 특화 메서드

void BoundingBoxDetection::setDetectionMethod(DetectionMethod method) {
    m_method = method;
}

BoundingBoxDetection::DetectionMethod BoundingBoxDetection::getDetectionMethod() const {
    return m_method;
}

void BoundingBoxDetection::setTemplateImage(const cv::Mat& templateImage) {
    if (!templateImage.empty()) {
        m_template = templateImage.clone();
    }
}

cv::Mat BoundingBoxDetection::getTemplateImage() const {
    return m_template;
}

void BoundingBoxDetection::setTargetColors(const std::vector<cv::Scalar>& colors) {
    m_targetColors = colors;
}

std::vector<cv::Scalar> BoundingBoxDetection::getTargetColors() const {
    return m_targetColors;
}

void BoundingBoxDetection::setMinObjectSize(const cv::Size& size) {
    m_minSize = size;
}

cv::Size BoundingBoxDetection::getMinObjectSize() const {
    return m_minSize;
}

void BoundingBoxDetection::setMaxObjectSize(const cv::Size& size) {
    m_maxSize = size;
}

cv::Size BoundingBoxDetection::getMaxObjectSize() const {
    return m_maxSize;
}

void BoundingBoxDetection::setUseMultiScale(bool useMultiScale) {
    m_useMultiScale = useMultiScale;
}

bool BoundingBoxDetection::getUseMultiScale() const {
    return m_useMultiScale;
}

// Private 메서드

std::vector<cv::Rect> BoundingBoxDetection::detectByTemplateMatching(const cv::Mat& image) {
    std::vector<cv::Rect> boxes;
    
    if (m_template.empty()) {
        std::cerr << "Template image not set for template matching" << std::endl;
        return boxes;
    }
    
    cv::Mat result;
    
    if (m_useMultiScale) {
        // 멀티스케일 템플릿 매칭
        for (double scale = 1.0; scale <= 3.0; scale *= m_scaleFactor) {
            cv::Mat resizedTemplate;
            cv::resize(m_template, resizedTemplate, cv::Size(), scale, scale, cv::INTER_LINEAR);
            
            if (resizedTemplate.cols > image.cols || resizedTemplate.rows > image.rows) {
                break;
            }
            
            cv::matchTemplate(image, resizedTemplate, result, cv::TM_CCOEFF_NORMED);
            
            double minVal, maxVal;
            cv::Point minLoc, maxLoc;
            cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
            
            if (maxVal >= m_confidenceThreshold) {
                cv::Rect box(maxLoc.x, maxLoc.y, resizedTemplate.cols, resizedTemplate.rows);
                boxes.push_back(box);
            }
        }
    } else {
        // 단일 스케일 템플릿 매칭
        cv::matchTemplate(image, m_template, result, cv::TM_CCOEFF_NORMED);
        
        double threshold = m_confidenceThreshold * 255.0;
        while (true) {
            double minVal, maxVal;
            cv::Point minLoc, maxLoc;
            cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
            
            if (maxVal < threshold) {
                break;
            }
            
            cv::Rect box(maxLoc.x, maxLoc.y, m_template.cols, m_template.rows);
            boxes.push_back(box);
            
            // 발견된 영역 제거
            cv::floodFill(result, maxLoc, cv::Scalar(0), cv::Scalar(255));
        }
    }
    
    return boxes;
}

std::vector<cv::Rect> BoundingBoxDetection::detectByContours(const cv::Mat& image) {
    std::vector<cv::Rect> boxes;
    
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // 가우시안 블러로 노이즈 제거
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
    
    // 이진화
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);
    
    // 컨투어 찾기
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(binary, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // 컨투어를 바운딩박스로 변환
    for (const auto& contour : contours) {
        cv::Rect box = cv::boundingRect(contour);
        
        // 크기 필터링
        if (box.width >= m_minSize.width && box.height >= m_minSize.height &&
            box.width <= m_maxSize.width && box.height <= m_maxSize.height) {
            boxes.push_back(box);
        }
    }
    
    return boxes;
}

std::vector<cv::Rect> BoundingBoxDetection::detectByEdges(const cv::Mat& image) {
    std::vector<cv::Rect> boxes;
    
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    // Canny 에지 감지
    cv::Mat edges;
    cv::Canny(gray, edges, 50, 150);
    
    // 팽창
    cv::Mat dilated;
    cv::dilate(edges, dilated, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));
    
    // 컨투어 찾기
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(dilated, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    // 컨투어를 바운딩박스로 변환
    for (const auto& contour : contours) {
        cv::Rect box = cv::boundingRect(contour);
        
        if (box.width >= m_minSize.width && box.height >= m_minSize.height &&
            box.width <= m_maxSize.width && box.height <= m_maxSize.height) {
            boxes.push_back(box);
        }
    }
    
    return boxes;
}

std::vector<cv::Rect> BoundingBoxDetection::detectByColorSegmentation(const cv::Mat& image) {
    std::vector<cv::Rect> boxes;
    
    if (m_targetColors.empty()) {
        std::cerr << "No target colors set for color segmentation" << std::endl;
        return boxes;
    }
    
    cv::Mat hsv;
    cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
    
    for (const cv::Scalar& targetColor : m_targetColors) {
        // 색상 범위 설정 (HSV)
        cv::Scalar lower(targetColor[0] - 10, targetColor[1] - 50, targetColor[2] - 50);
        cv::Scalar upper(targetColor[0] + 10, targetColor[1] + 50, targetColor[2] + 50);
        
        // 색상 마스크 생성
        cv::Mat mask;
        cv::inRange(hsv, lower, upper, mask);
        
        // 모폴로지 연산으로 노이즈 제거
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
        cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
        
        // 컨투어 찾기
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        
        // 컨투어를 바운딩박스로 변환
        for (const auto& contour : contours) {
            cv::Rect box = cv::boundingRect(contour);
            
            if (box.width >= m_minSize.width && box.height >= m_minSize.height &&
                box.width <= m_maxSize.width && box.height <= m_maxSize.height) {
                boxes.push_back(box);
            }
        }
    }
    
    return boxes;
}

std::vector<int> BoundingBoxDetection::applyNMS(const std::vector<cv::Rect>& boxes, 
                                            const std::vector<double>& scores, 
                                            double threshold) {
    std::vector<int> indices(boxes.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    // 신뢰도 순으로 정렬
    std::sort(indices.begin(), indices.end(), 
        [&scores](int i1, int i2) { return scores[i1] > scores[i2]; });
    
    std::vector<bool> suppressed(boxes.size(), false);
    std::vector<int> keep;
    
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        
        if (suppressed[idx]) {
            continue;
        }
        
        keep.push_back(idx);
        
        // 현재 박스와 겹치는 다른 박스들 억제
        for (size_t j = i + 1; j < indices.size(); ++j) {
            int idx2 = indices[j];
            
            if (suppressed[idx2]) {
                continue;
            }
            
            // IoU (Intersection over Union) 계산
            cv::Rect intersection = boxes[idx] & boxes[idx2];
            cv::Rect unionBox = boxes[idx] | boxes[idx2];
            
            double iou = static_cast<double>(intersection.area()) / unionBox.area();
            
            if (iou > threshold) {
                suppressed[idx2] = true;
            }
        }
    }
    
    return keep;
}

std::vector<cv::Rect> BoundingBoxDetection::filterBoundingBoxes(const std::vector<cv::Rect>& boxes) {
    std::vector<cv::Rect> filtered;
    
    // 크기 필터링
    for (const cv::Rect& box : boxes) {
        if (box.width >= m_minSize.width && box.height >= m_minSize.height &&
            box.width <= m_maxSize.width && box.height <= m_maxSize.height) {
            filtered.push_back(box);
        }
    }
    
    return filtered;
}

cv::Mat BoundingBoxDetection::preprocessImage(const cv::Mat& image) {
    cv::Mat processed;
    
    // 노이즈 제거를 위한 가우시안 블러
    cv::GaussianBlur(image, processed, cv::Size(3, 3), 0);
    
    return processed;
}

void BoundingBoxDetection::setupPreprocessingParameters() {
    // 전처리 파라미터 설정
    // 필요에 따라 동적으로 조정 가능
}
