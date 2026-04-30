#include "processing/ProcessingPipeline.h"
#include <iostream>
#include <thread>
#include <chrono>

ProcessingPipeline::ProcessingPipeline(bool saveIntermediateResults) 
    : m_saveIntermediateResults(saveIntermediateResults), m_enabled(true) {
}

ProcessingPipeline::~ProcessingPipeline() {
    cleanup();
}

int ProcessingPipeline::addAlgorithm(std::unique_ptr<IProcessingAlgorithm> algorithm) {
    if (!algorithm) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    // 알고리즘 초기화
    if (!algorithm->initialize()) {
        std::cerr << "Failed to initialize algorithm: " << algorithm->getName() << std::endl;
        return -1;
    }
    
    int index = static_cast<int>(m_pipeline.size());
    m_pipeline.push_back(std::move(algorithm));
    
    std::cout << "Added algorithm to pipeline: " << m_pipeline.back()->getName() 
              << " at index " << index << std::endl;
    
    return index;
}

bool ProcessingPipeline::removeAlgorithm(int index) {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    if (index < 0 || index >= static_cast<int>(m_pipeline.size())) {
        std::cerr << "Invalid algorithm index: " << index << std::endl;
        return false;
    }
    
    std::string algorithmName = m_pipeline[index]->getName();
    m_pipeline[index]->cleanup();
    m_pipeline.erase(m_pipeline.begin() + index);
    
    std::cout << "Removed algorithm from pipeline: " << algorithmName << std::endl;
    return true;
}

bool ProcessingPipeline::removeAlgorithm(const std::string& algorithmName) {
    int index = findAlgorithmIndex(algorithmName);
    if (index == -1) {
        std::cerr << "Algorithm not found: " << algorithmName << std::endl;
        return false;
    }
    
    return removeAlgorithm(index);
}

IProcessingAlgorithm* ProcessingPipeline::getAlgorithm(const std::string& algorithmName) const {
    int index = findAlgorithmIndex(algorithmName);
    if (index == -1) {
        return nullptr;
    }
    
    return m_pipeline[index].get();
}

ProcessingResult ProcessingPipeline::execute(const cv::Mat& input) {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    if (!m_enabled) {
        ProcessingResult result;
        result.success = false;
        result.errorMessage = "Pipeline is disabled";
        return result;
    }
    
    if (m_pipeline.empty()) {
        ProcessingResult result;
        result.success = false;
        result.errorMessage = "Pipeline is empty";
        return result;
    }
    
    // 중간 결과 비우기
    if (m_saveIntermediateResults) {
        clearIntermediateResults();
    }
    
    cv::Mat currentInput = input.clone();
    ProcessingResult finalResult;
    finalResult.success = true;
    
    // 파이프라인의 모든 알고리즘 순차 실행
    for (size_t i = 0; i < m_pipeline.size(); ++i) {
        auto& algorithm = m_pipeline[i];
        
        std::cout << "Executing algorithm " << i << ": " << algorithm->getName() << std::endl;
        
        ProcessingResult result = executeAlgorithm(algorithm.get(), currentInput);
        
        if (m_saveIntermediateResults) {
            saveIntermediateResult(result);
        }
        
        if (!result.success) {
            std::cerr << "Algorithm failed: " << algorithm->getName() 
                      << " - " << result.errorMessage << std::endl;
            return result;
        }
        
        // 다음 알고리즘의 입력으로 현재 결과 사용
        currentInput = result.processedImage.clone();
        finalResult = result; // 마지막 결과 저장
    }
    
    finalResult.algorithmName = "Pipeline";
    return finalResult;
}

std::future<ProcessingResult> ProcessingPipeline::executeAsync(const cv::Mat& input, 
                                                        std::function<void(const ProcessingResult&)> callback) {
    auto promise = std::make_shared<std::promise<ProcessingResult>>();
    auto future = promise->get_future();
    
    // 비동기 실행
    std::thread([this, input, promise, callback]() {
        ProcessingResult result = execute(input);
        
        // 콜백 호출
        if (callback) {
            callback(result);
        }
        
        promise->set_value(result);
    }).detach();
    
    return future;
}

ProcessingResult ProcessingPipeline::executeFrom(const cv::Mat& input, int startIndex) {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    if (startIndex < 0 || startIndex >= static_cast<int>(m_pipeline.size())) {
        ProcessingResult result;
        result.success = false;
        result.errorMessage = "Invalid start index";
        return result;
    }
    
    cv::Mat currentInput = input.clone();
    ProcessingResult finalResult;
    finalResult.success = true;
    
    // startIndex부터 실행
    for (size_t i = startIndex; i < m_pipeline.size(); ++i) {
        auto& algorithm = m_pipeline[i];
        
        ProcessingResult result = executeAlgorithm(algorithm.get(), currentInput);
        
        if (!result.success) {
            return result;
        }
        
        currentInput = result.processedImage.clone();
        finalResult = result;
    }
    
    return finalResult;
}

ProcessingResult ProcessingPipeline::executeRange(const cv::Mat& input, int startIndex, int endIndex) {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    if (startIndex < 0 || startIndex >= static_cast<int>(m_pipeline.size()) ||
        endIndex < 0 || endIndex > static_cast<int>(m_pipeline.size()) ||
        startIndex >= endIndex) {
        
        ProcessingResult result;
        result.success = false;
        result.errorMessage = "Invalid range";
        return result;
    }
    
    cv::Mat currentInput = input.clone();
    ProcessingResult finalResult;
    finalResult.success = true;
    
    // 지정된 범위 실행
    for (int i = startIndex; i < endIndex; ++i) {
        auto& algorithm = m_pipeline[i];
        
        ProcessingResult result = executeAlgorithm(algorithm.get(), currentInput);
        
        if (!result.success) {
            return result;
        }
        
        currentInput = result.processedImage.clone();
        finalResult = result;
    }
    
    return finalResult;
}

bool ProcessingPipeline::initialize() {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    bool allInitialized = true;
    for (const auto& algorithm : m_pipeline) {
        if (!algorithm->initialize()) {
            std::cerr << "Failed to initialize algorithm: " << algorithm->getName() << std::endl;
            allInitialized = false;
        }
    }
    
    return allInitialized;
}

void ProcessingPipeline::cleanup() {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    for (const auto& algorithm : m_pipeline) {
        algorithm->cleanup();
    }
    
    clearIntermediateResults();
}

void ProcessingPipeline::clear() {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    // 모든 알고리즘 종료
    for (const auto& algorithm : m_pipeline) {
        algorithm->cleanup();
    }
    
    m_pipeline.clear();
    clearIntermediateResults();
}

void ProcessingPipeline::setEnabled(bool enabled) {
    m_enabled = enabled;
}

void ProcessingPipeline::setSaveIntermediateResults(bool save) {
    m_saveIntermediateResults = save;
}

size_t ProcessingPipeline::size() const {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    return m_pipeline.size();
}

bool ProcessingPipeline::empty() const {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    return m_pipeline.empty();
}

std::vector<std::string> ProcessingPipeline::getAlgorithmNames() const {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    std::vector<std::string> names;
    names.reserve(m_pipeline.size());
    
    for (const auto& algorithm : m_pipeline) {
        names.push_back(algorithm->getName());
    }
    
    return names;
}

std::vector<ProcessingResult> ProcessingPipeline::getIntermediateResults() const {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    return m_intermediateResults;
}

ProcessingResult ProcessingPipeline::getAlgorithmResult(const std::string& algorithmName) const {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    for (const auto& result : m_intermediateResults) {
        if (result.algorithmName == algorithmName) {
            return result;
        }
    }
    
    ProcessingResult notFound;
    notFound.success = false;
    notFound.errorMessage = "Algorithm result not found: " + algorithmName;
    return notFound;
}

std::string ProcessingPipeline::getPipelineInfo() const {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    std::ostringstream oss;
    oss << "Processing Pipeline Info:\n";
    oss << "  Algorithms: " << m_pipeline.size() << "\n";
    oss << "  Enabled: " << (m_enabled ? "Yes" : "No") << "\n";
    oss << "  Save Intermediate: " << (m_saveIntermediateResults ? "Yes" : "No") << "\n";
    oss << "  Pipeline:\n";
    
    for (size_t i = 0; i < m_pipeline.size(); ++i) {
        const auto& algorithm = m_pipeline[i];
        oss << "    " << i << ". " << algorithm->getName() 
             << " (" << algorithm->getAlgorithmType() << ")\n";
    }
    
    return oss.str();
}

std::unique_ptr<ProcessingPipeline> ProcessingPipeline::clone() const {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    auto newPipeline = std::make_unique<ProcessingPipeline>(m_saveIntermediateResults);
    
    for (const auto& algorithm : m_pipeline) {
        // TODO: 알고리즘 복제 기능 필요
        // 현재는 이름만 복사
        std::cout << "Warning: Algorithm cloning not implemented for: " << algorithm->getName() << std::endl;
    }
    
    return newPipeline;
}

bool ProcessingPipeline::swapAlgorithms(int fromIndex, int toIndex) {
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    if (fromIndex < 0 || fromIndex >= static_cast<int>(m_pipeline.size()) ||
        toIndex < 0 || toIndex >= static_cast<int>(m_pipeline.size())) {
        return false;
    }
    
    std::swap(m_pipeline[fromIndex], m_pipeline[toIndex]);
    return true;
}

bool ProcessingPipeline::insertAlgorithm(std::unique_ptr<IProcessingAlgorithm> algorithm, int index) {
    if (!algorithm) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_pipelineMutex);
    
    if (index < 0 || index > static_cast<int>(m_pipeline.size())) {
        return false;
    }
    
    if (!algorithm->initialize()) {
        std::cerr << "Failed to initialize algorithm: " << algorithm->getName() << std::endl;
        return false;
    }
    
    m_pipeline.insert(m_pipeline.begin() + index, std::move(algorithm));
    return true;
}

// Private 메서드

int ProcessingPipeline::findAlgorithmIndex(const std::string& algorithmName) const {
    for (size_t i = 0; i < m_pipeline.size(); ++i) {
        if (m_pipeline[i]->getName() == algorithmName) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

ProcessingResult ProcessingPipeline::executeAlgorithm(IProcessingAlgorithm* algorithm, const cv::Mat& input) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    ProcessingResult result;
    result.algorithmName = algorithm->getName();
    
    try {
        // 이미지 처리 가능 여부 확인
        if (!algorithm->canProcess(input.type())) {
            result.success = false;
            result.errorMessage = "Algorithm cannot process this image type";
            return result;
        }
        
        // 알고리즘 실행
        result = algorithm->process(input);
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Algorithm execution error: " + std::string(e.what());
        std::cerr << "Algorithm execution error: " << e.what() << std::endl;
        
    } catch (...) {
        result.success = false;
        result.errorMessage = "Unknown algorithm execution error";
        std::cerr << "Unknown algorithm execution error" << std::endl;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    return result;
}

void ProcessingPipeline::saveIntermediateResult(const ProcessingResult& result) {
    m_intermediateResults.push_back(result);
}

void ProcessingPipeline::clearIntermediateResults() {
    m_intermediateResults.clear();
}
