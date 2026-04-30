#include "processing/algorithms/MatchingAlgorithm.h"
#include <iostream>
#include <algorithm>
#include <cmath>

MatchingAlgorithm::MatchingAlgorithm() 
    : m_method(MatchingMethod::TemplateMatching),
      m_matchThreshold(0.7),
      m_templateMatchingMethod(cv::TM_CCOEFF_NORMED),
      m_featureDetectorType("ORB"),
      m_featureMatcherType("BFMatcher"),
      m_useMultiScale(true),
      m_scaleRangeMin(0.5),
      m_scaleRangeMax(2.0),
      m_scaleStep(0.1),
      m_useRotationInvariant(false) {
}

std::string MatchingAlgorithm::getName() const {
    return "Image Matching";
}

std::string MatchingAlgorithm::getDescription() const {
    return "Matches images using various methods including template matching and feature matching";
}

ProcessingResult MatchingAlgorithm::process(const cv::Mat& input) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    ProcessingResult result;
    result.algorithmName = getName();
    result.success = false;
    
    if (input.empty()) {
        result.errorMessage = "Input image is empty";
        return result;
    }
    
    if (m_template.empty() && m_templates.empty()) {
        result.errorMessage = "No template image set";
        return result;
    }
    
    try {
        // 이미지 전처리
        cv::Mat processedImage = preprocessImage(input);
        m_matchResults.clear();
        
        // 단일 템플릿 또는 다중 템플릿 처리
        std::vector<cv::Mat> templatesToMatch = m_templates;
        if (!m_template.empty()) {
            templatesToMatch.insert(templatesToMatch.begin(), m_template);
        }
        
        for (const auto& templateImage : templatesToMatch) {
            if (templateImage.empty()) continue;
            
            MatchResult matchResult;
            
            switch (m_method) {
                case MatchingMethod::TemplateMatching:
                    matchResult = performTemplateMatching(processedImage, templateImage);
                    break;
                    
                case MatchingMethod::FeatureMatching:
                    matchResult = performFeatureMatching(processedImage, templateImage);
                    break;
                    
                case MatchingMethod::HistogramMatching:
                    matchResult = performHistogramMatching(processedImage, templateImage);
                    break;
                    
                case MatchingMethod::StructuralSimilarity:
                    matchResult.similarity = calculateSSIM(processedImage, templateImage);
                    matchResult.confidence = matchResult.similarity;
                    break;
                    
                default:
                    result.errorMessage = "Unknown matching method";
                    return result;
            }
            
            m_matchResults.push_back(matchResult);
        }
        
        // 결과 설정
        result.processedImage = processedImage.clone();
        result.success = true;
        
        std::cout << "Found " << m_matchResults.size() << " matches" << std::endl;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Matching error: " + std::string(e.what());
        std::cerr << "Image matching error: " << e.what() << std::endl;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    return result;
}

bool MatchingAlgorithm::supportsAsyncProcessing() const {
    return true;
}

void MatchingAlgorithm::setParameter(const std::string& name, double value) {
    m_parameters[name] = value;
    
    // 내부 변수 업데이트
    if (name == "match_threshold") {
        m_matchThreshold = value;
    } else if (name == "template_matching_method") {
        m_templateMatchingMethod = static_cast<int>(value);
    } else if (name == "use_multiscale") {
        m_useMultiScale = (value != 0.0);
    } else if (name == "scale_range_min") {
        m_scaleRangeMin = value;
    } else if (name == "scale_range_max") {
        m_scaleRangeMax = value;
    } else if (name == "scale_step") {
        m_scaleStep = value;
    } else if (name == "use_rotation_invariant") {
        m_useRotationInvariant = (value != 0.0);
    }
}

double MatchingAlgorithm::getParameter(const std::string& name) const {
    return getParameterSafe(name);
}

std::vector<std::string> MatchingAlgorithm::getSupportedParameters() const {
    return {
        "match_threshold",
        "template_matching_method",
        "use_multiscale",
        "scale_range_min",
        "scale_range_max",
        "scale_step",
        "use_rotation_invariant"
    };
}

bool MatchingAlgorithm::initialize() {
    m_initialized = true;
    return true;
}

void MatchingAlgorithm::cleanup() {
    m_matchResults.clear();
    m_template.release();
    m_templates.clear();
    m_initialized = false;
}

bool MatchingAlgorithm::isInitialized() const {
    return m_initialized;
}

bool MatchingAlgorithm::canProcess(int imageType) const {
    // 그레이스케일 또는 컬러 이미지 지원
    return (imageType == CV_8UC1) || (imageType == CV_8UC3) || (imageType == CV_8UC4);
}

std::string MatchingAlgorithm::getAlgorithmType() const {
    return "Image Matching";
}

// 특화 메서드

void MatchingAlgorithm::setMatchingMethod(MatchingMethod method) {
    m_method = method;
}

MatchingAlgorithm::MatchingMethod MatchingAlgorithm::getMatchingMethod() const {
    return m_method;
}

void MatchingAlgorithm::setTemplate(const cv::Mat& templateImage) {
    if (!templateImage.empty()) {
        m_template = templateImage.clone();
    }
}

cv::Mat MatchingAlgorithm::getTemplate() const {
    return m_template;
}

void MatchingAlgorithm::addTemplate(const cv::Mat& templateImage) {
    if (!templateImage.empty()) {
        m_templates.push_back(templateImage.clone());
    }
}

void MatchingAlgorithm::clearTemplates() {
    m_templates.clear();
}

std::vector<cv::Mat> MatchingAlgorithm::getTemplates() const {
    return m_templates;
}

void MatchingAlgorithm::setMatchThreshold(double threshold) {
    m_matchThreshold = threshold;
}

double MatchingAlgorithm::getMatchThreshold() const {
    return m_matchThreshold;
}

void MatchingAlgorithm::setTemplateMatchingMethod(int method) {
    m_templateMatchingMethod = method;
}

int MatchingAlgorithm::getTemplateMatchingMethod() const {
    return m_templateMatchingMethod;
}

void MatchingAlgorithm::setFeatureDetector(const std::string& detectorType) {
    m_featureDetectorType = detectorType;
}

void MatchingAlgorithm::setFeatureMatcher(const std::string& matcherType) {
    m_featureMatcherType = matcherType;
}

std::string MatchingAlgorithm::getFeatureDetectorType() const {
    return m_featureDetectorType;
}

std::string MatchingAlgorithm::getFeatureMatcherType() const {
    return m_featureMatcherType;
}

void MatchingAlgorithm::setMultiScaleParameters(bool useMultiScale, double minScale, double maxScale, double step) {
    m_useMultiScale = useMultiScale;
    m_scaleRangeMin = minScale;
    m_scaleRangeMax = maxScale;
    m_scaleStep = step;
}

void MatchingAlgorithm::getMultiScaleParameters(bool* useMultiScale, double* minScale, double* maxScale, double* step) const {
    if (useMultiScale) *useMultiScale = m_useMultiScale;
    if (minScale) *minScale = m_scaleRangeMin;
    if (maxScale) *maxScale = m_scaleRangeMax;
    if (step) *step = m_scaleStep;
}

void MatchingAlgorithm::setRotationInvariant(bool useRotationInvariant) {
    m_useRotationInvariant = useRotationInvariant;
}

bool MatchingAlgorithm::getRotationInvariant() const {
    return m_useRotationInvariant;
}

std::vector<MatchingAlgorithm::MatchResult> MatchingAlgorithm::getMatchResults() const {
    return m_matchResults;
}

MatchingAlgorithm::MatchResult MatchingAlgorithm::getBestMatch() const {
    if (m_matchResults.empty()) {
        return MatchResult();
    }
    
    auto bestMatch = std::max_element(m_matchResults.begin(), m_matchResults.end(),
        [](const MatchResult& a, const MatchResult& b) {
            return a.confidence < b.confidence;
        });
    
    return *bestMatch;
}

// Private 메서드

MatchingAlgorithm::MatchResult MatchingAlgorithm::performTemplateMatching(const cv::Mat& image, const cv::Mat& templateImage) {
    MatchResult result;
    
    if (m_useMultiScale) {
        result = performMultiScaleMatching(image, templateImage);
    } else if (m_useRotationInvariant) {
        result = performRotationInvariantMatching(image, templateImage);
    } else {
        // 단일 스케일 템플릿 매칭
        cv::Mat resultImage;
        cv::matchTemplate(image, templateImage, resultImage, m_templateMatchingMethod);
        
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(resultImage, &minVal, &maxVal, &minLoc, &maxLoc);
        
        if (m_templateMatchingMethod == cv::TM_SQDIFF || m_templateMatchingMethod == cv::TM_SQDIFF_NORMED) {
            result.location = minLoc;
            result.confidence = 1.0 - (minVal / 255.0);
        } else {
            result.location = maxLoc;
            result.confidence = maxVal;
        }
        
        result.boundingBox = cv::Rect(result.location, templateImage.size());
        result.similarity = result.confidence;
    }
    
    return result;
}

MatchingAlgorithm::MatchResult MatchingAlgorithm::performFeatureMatching(const cv::Mat& image, const cv::Mat& templateImage) {
    MatchResult result;
    
    try {
        // 특징점 추출
        std::vector<cv::KeyPoint> keypoints1 = extractFeatures(image, m_featureDetectorType);
        std::vector<cv::KeyPoint> keypoints2 = extractFeatures(templateImage, m_featureDetectorType);
        
        if (keypoints1.empty() || keypoints2.empty()) {
            return result;
        }
        
        // 기술자 계산
        cv::Mat descriptors1 = computeDescriptors(image, keypoints1, m_featureDetectorType);
        cv::Mat descriptors2 = computeDescriptors(templateImage, keypoints2, m_featureDetectorType);
        
        if (descriptors1.empty() || descriptors2.empty()) {
            return result;
        }
        
        // 특징점 매칭
        std::vector<cv::DMatch> matches = matchFeatures(descriptors1, descriptors2, m_featureMatcherType);
        
        // 매치 필터링
        matches = filterMatches(keypoints1, keypoints2, matches);
        
        if (!matches.empty()) {
            result.keypoints = keypoints2;
            result.matches = matches;
            result.confidence = static_cast<double>(matches.size()) / std::max(keypoints1.size(), keypoints2.size());
            result.similarity = result.confidence;
            
            // 매칭 위치 계산 (가장 좋은 매치 기준)
            cv::Point2f center(0, 0);
            for (const auto& match : matches) {
                center += keypoints1[match.queryIdx].pt;
            }
            center *= 1.0f / matches.size();
            result.location = cv::Point(static_cast<int>(center.x), static_cast<int>(center.y));
            result.boundingBox = cv::Rect(result.location, templateImage.size());
        }
        
    } catch (const cv::Exception& e) {
        std::cerr << "OpenCV error in feature matching: " << e.what() << std::endl;
    }
    
    return result;
}

MatchingAlgorithm::MatchResult MatchingAlgorithm::performHistogramMatching(const cv::Mat& image, const cv::Mat& templateImage) {
    MatchResult result;
    
    // 히스토그램 계산
    cv::Mat hist1 = calculateHistogram(image);
    cv::Mat hist2 = calculateHistogram(templateImage);
    
    // 히스토그램 비교
    double similarity = compareHistograms(hist1, hist2, cv::HISTCMP_CORREL);
    
    result.similarity = similarity;
    result.confidence = similarity;
    
    // 히스토그램 매칭은 위치 정보를 제공하지 않음
    result.location = cv::Point(0, 0);
    result.boundingBox = cv::Rect(0, 0, templateImage.cols, templateImage.rows);
    
    return result;
}

double MatchingAlgorithm::calculateSSIM(const cv::Mat& image, const cv::Mat& templateImage) {
    // SSIM 계산을 위한 전처리
    cv::Mat img1, img2;
    if (image.channels() == 3) {
        cv::cvtColor(image, img1, cv::COLOR_BGR2GRAY);
    } else {
        img1 = image.clone();
    }
    
    if (templateImage.channels() == 3) {
        cv::cvtColor(templateImage, img2, cv::COLOR_BGR2GRAY);
    } else {
        img2 = templateImage.clone();
    }
    
    // 크기 맞추기
    if (img1.size() != img2.size()) {
        cv::resize(img2, img2, img1.size());
    }
    
    // SSIM 계산 (간단한 버전)
    cv::Scalar mu1, mu2;
    cv::Scalar sigma1, sigma2, sigma12;
    
    cv::Mat img1_f, img2_f;
    img1.convertTo(img1_f, CV_32F);
    img2.convertTo(img2_f, CV_32F);
    
    mu1 = cv::mean(img1_f);
    mu2 = cv::mean(img2_f);
    
    cv::Mat img1_sq, img2_sq, img1_img2;
    cv::multiply(img1_f, img1_f, img1_sq);
    cv::multiply(img2_f, img2_f, img2_sq);
    cv::multiply(img1_f, img2_f, img1_img2);
    
    sigma1.val[0] = cv::sqrt(cv::mean(img1_sq).val[0] - mu1.val[0] * mu1.val[0]);
    sigma2.val[0] = cv::sqrt(cv::mean(img2_sq).val[0] - mu2.val[0] * mu2.val[0]);
    sigma12.val[0] = cv::mean(img1_img2).val[0] - mu1.val[0] * mu2.val[0];
    
    const double C1 = 6.5025, C2 = 58.5225;
    double ssim = ((2 * mu1.val[0] * mu2.val[0] + C1) * (2 * sigma12.val[0] + C2)) /
                  ((mu1.val[0] * mu1.val[0] + mu2.val[0] * mu2.val[0] + C1) * 
                   (sigma1.val[0] * sigma1.val[0] + sigma2.val[0] * sigma2.val[0] + C2));
    
    return ssim;
}

MatchingAlgorithm::MatchResult MatchingAlgorithm::performMultiScaleMatching(const cv::Mat& image, const cv::Mat& templateImage) {
    MatchResult bestResult;
    
    for (double scale = m_scaleRangeMin; scale <= m_scaleRangeMax; scale += m_scaleStep) {
        cv::Mat scaledTemplate;
        cv::resize(templateImage, scaledTemplate, cv::Size(), scale, scale, cv::INTER_LINEAR);
        
        if (scaledTemplate.cols > image.cols || scaledTemplate.rows > image.rows) {
            continue;
        }
        
        cv::Mat resultImage;
        cv::matchTemplate(image, scaledTemplate, resultImage, m_templateMatchingMethod);
        
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(resultImage, &minVal, &maxVal, &minLoc, &maxLoc);
        
        double confidence;
        if (m_templateMatchingMethod == cv::TM_SQDIFF || m_templateMatchingMethod == cv::TM_SQDIFF_NORMED) {
            confidence = 1.0 - (minVal / 255.0);
        } else {
            confidence = maxVal;
        }
        
        if (confidence > bestResult.confidence) {
            bestResult.location = (m_templateMatchingMethod == cv::TM_SQDIFF || m_templateMatchingMethod == cv::TM_SQDIFF_NORMED) ? minLoc : maxLoc;
            bestResult.confidence = confidence;
            bestResult.similarity = confidence;
            bestResult.boundingBox = cv::Rect(bestResult.location, scaledTemplate.size());
        }
    }
    
    return bestResult;
}

MatchingAlgorithm::MatchResult MatchingAlgorithm::performRotationInvariantMatching(const cv::Mat& image, const cv::Mat& templateImage) {
    MatchResult bestResult;
    
    // 회전 각도 범위 (0-360도, 15도 간격)
    for (double angle = 0; angle < 360; angle += 15) {
        cv::Mat rotatedTemplate;
        cv::Point2f center(templateImage.cols / 2.0f, templateImage.rows / 2.0f);
        cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, angle, 1.0);
        cv::warpAffine(templateImage, rotatedTemplate, rotationMatrix, templateImage.size());
        
        cv::Mat resultImage;
        cv::matchTemplate(image, rotatedTemplate, resultImage, m_templateMatchingMethod);
        
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(resultImage, &minVal, &maxVal, &minLoc, &maxLoc);
        
        double confidence;
        if (m_templateMatchingMethod == cv::TM_SQDIFF || m_templateMatchingMethod == cv::TM_SQDIFF_NORMED) {
            confidence = 1.0 - (minVal / 255.0);
        } else {
            confidence = maxVal;
        }
        
        if (confidence > bestResult.confidence) {
            bestResult.location = (m_templateMatchingMethod == cv::TM_SQDIFF || m_templateMatchingMethod == cv::TM_SQDIFF_NORMED) ? minLoc : maxLoc;
            bestResult.confidence = confidence;
            bestResult.similarity = confidence;
            bestResult.boundingBox = cv::Rect(bestResult.location, rotatedTemplate.size());
        }
    }
    
    return bestResult;
}

std::vector<cv::KeyPoint> MatchingAlgorithm::extractFeatures(const cv::Mat& image, const std::string& detectorType) {
    std::vector<cv::KeyPoint> keypoints;
    
    try {
        cv::Ptr<cv::FeatureDetector> detector;
        
        if (detectorType == "ORB") {
            detector = cv::ORB::create();
        } else if (detectorType == "SIFT") {
            detector = cv::SIFT::create();
        } else if (detectorType == "SURF") {
            detector = cv::xfeatures2d::SURF::create();
        } else if (detectorType == "AKAZE") {
            detector = cv::AKAZE::create();
        } else {
            detector = cv::ORB::create(); // 기본값
        }
        
        detector->detect(image, keypoints);
        
    } catch (const cv::Exception& e) {
        std::cerr << "Feature extraction error: " << e.what() << std::endl;
    }
    
    return keypoints;
}

cv::Mat MatchingAlgorithm::computeDescriptors(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, const std::string& detectorType) {
    cv::Mat descriptors;
    
    try {
        cv::Ptr<cv::DescriptorExtractor> extractor;
        
        if (detectorType == "ORB") {
            extractor = cv::ORB::create();
        } else if (detectorType == "SIFT") {
            extractor = cv::SIFT::create();
        } else if (detectorType == "SURF") {
            extractor = cv::xfeatures2d::SURF::create();
        } else if (detectorType == "AKAZE") {
            extractor = cv::AKAZE::create();
        } else {
            extractor = cv::ORB::create();
        }
        
        extractor->compute(image, keypoints, descriptors);
        
    } catch (const cv::Exception& e) {
        std::cerr << "Descriptor computation error: " << e.what() << std::endl;
    }
    
    return descriptors;
}

std::vector<cv::DMatch> MatchingAlgorithm::matchFeatures(const cv::Mat& descriptors1, const cv::Mat& descriptors2, const std::string& matcherType) {
    std::vector<cv::DMatch> matches;
    
    try {
        if (matcherType == "BFMatcher") {
            cv::BFMatcher matcher(cv::NORM_L2);
            matcher.match(descriptors1, descriptors2, matches);
        } else if (matcherType == "FlannBased") {
            cv::FlannBasedMatcher matcher;
            matcher.match(descriptors1, descriptors2, matches);
        } else {
            cv::BFMatcher matcher(cv::NORM_L2);
            matcher.match(descriptors1, descriptors2, matches);
        }
        
    } catch (const cv::Exception& e) {
        std::cerr << "Feature matching error: " << e.what() << std::endl;
    }
    
    return matches;
}

std::vector<cv::DMatch> MatchingAlgorithm::filterMatches(const std::vector<cv::KeyPoint>& keypoints1,
                                                        const std::vector<cv::KeyPoint>& keypoints2,
                                                        const std::vector<cv::DMatch>& matches) {
    std::vector<cv::DMatch> goodMatches;
    
    if (matches.empty()) {
        return goodMatches;
    }
    
    // 거리 기반 필터링
    double minDist = std::min_element(matches.begin(), matches.end(),
        [](const cv::DMatch& a, const cv::DMatch& b) { return a.distance < b.distance; })->distance;
    
    for (const auto& match : matches) {
        if (match.distance < std::max(2.0 * minDist, 30.0)) {
            goodMatches.push_back(match);
        }
    }
    
    return goodMatches;
}

cv::Mat MatchingAlgorithm::preprocessImage(const cv::Mat& image) {
    cv::Mat processed;
    
    // 노이즈 제거
    cv::GaussianBlur(image, processed, cv::Size(3, 3), 0);
    
    return processed;
}

cv::Mat MatchingAlgorithm::calculateHistogram(const cv::Mat& image, int bins) {
    cv::Mat hist;
    
    if (image.channels() == 3) {
        // 컬러 히스토그램
        std::vector<cv::Mat> channels;
        cv::split(image, channels);
        
        float range[] = {0, 256};
        const float* histRange = {range};
        
        cv::calcHist(&channels[0], 1, 0, cv::Mat(), hist, 1, &bins, &histRange);
        cv::normalize(hist, hist, 0, 1, cv::NORM_L1);
    } else {
        // 그레이스케일 히스토그램
        float range[] = {0, 256};
        const float* histRange = {range};
        
        cv::calcHist(&image, 1, 0, cv::Mat(), hist, 1, &bins, &histRange);
        cv::normalize(hist, hist, 0, 1, cv::NORM_L1);
    }
    
    return hist;
}

double MatchingAlgorithm::compareHistograms(const cv::Mat& hist1, const cv::Mat& hist2, int method) {
    return cv::compareHist(hist1, hist2, method);
}

MatchingAlgorithm::MatchResult MatchingAlgorithm::createMatchResult(const cv::Point& location, double confidence, double similarity, const cv::Size& templateSize) {
    MatchResult result;
    result.location = location;
    result.confidence = confidence;
    result.similarity = similarity;
    result.boundingBox = cv::Rect(location, templateSize);
    return result;
}
