#include "processing/ImageProcessor.h"
#include <iostream>
#include <chrono>

ImageProcessor::ImageProcessor(size_t maxThreads) 
    : m_shouldStop(false), m_isRunning(false), m_maxThreads(maxThreads),
      m_activeTasks(0), m_completedTasks(0) {
}

ImageProcessor::~ImageProcessor() {
    stop();
}

bool ImageProcessor::start() {
    if (m_isRunning.load()) {
        return true;
    }
    
    // 모든 알고리즘 초기화
    if (!initializeAlgorithms()) {
        std::cerr << "Failed to initialize algorithms" << std::endl;
        return false;
    }
    
    m_shouldStop.store(false);
    m_isRunning.store(true);
    
    // 작업자 스레드 생성
    size_t numThreads = std::min(m_maxThreads, static_cast<size_t>(std::thread::hardware_concurrency()));
    
    for (size_t i = 0; i < numThreads; ++i) {
        m_workerThreads.emplace_back(&ImageProcessor::workerThread, this);
    }
    
    std::cout << "ImageProcessor started with " << numThreads << " threads" << std::endl;
    return true;
}

void ImageProcessor::stop() {
    if (!m_isRunning.load()) {
        return;
    }
    
    m_shouldStop.store(true);
    
    // 모든 스레드 종료 대기
    for (auto& thread : m_workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    m_workerThreads.clear();
    m_isRunning.store(false);
    
    // 알고리즘 종료
    cleanupAlgorithms();
    
    std::cout << "ImageProcessor stopped" << std::endl;
}

bool ImageProcessor::registerAlgorithm(std::unique_ptr<IProcessingAlgorithm> algorithm) {
    if (!algorithm) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_algorithmsMutex);
    
    // 동일 이름의 알고리즘이 있는지 확인
    std::string name = algorithm->getName();
    for (const auto& existing : m_algorithms) {
        if (existing->getName() == name) {
            std::cerr << "Algorithm with name '" << name << "' already exists" << std::endl;
            return false;
        }
    }
    
    // 알고리즘 초기화
    if (!algorithm->initialize()) {
        std::cerr << "Failed to initialize algorithm: " << name << std::endl;
        return false;
    }
    
    m_algorithms.push_back(std::move(algorithm));
    std::cout << "Registered algorithm: " << name << std::endl;
    return true;
}

bool ImageProcessor::unregisterAlgorithm(const std::string& algorithmName) {
    std::lock_guard<std::mutex> lock(m_algorithmsMutex);
    
    auto it = std::find_if(m_algorithms.begin(), m_algorithms.end(),
        [&algorithmName](const std::unique_ptr<IProcessingAlgorithm>& algo) {
            return algo->getName() == algorithmName;
        });
    
    if (it != m_algorithms.end()) {
        (*it)->cleanup();
        m_algorithms.erase(it);
        std::cout << "Unregistered algorithm: " << algorithmName << std::endl;
        return true;
    }
    
    std::cerr << "Algorithm not found: " << algorithmName << std::endl;
    return false;
}

std::future<ProcessingResult> ImageProcessor::submitTask(const ProcessingTask& task) {
    auto promise = std::make_shared<std::promise<ProcessingResult>>();
    auto future = promise->get_future();
    
    // 콜백 수정하여 promise 설정
    ProcessingTask modifiedTask = task;
    modifiedTask.callback = [promise](const ProcessingResult& result) {
        promise->set_value(result);
    };
    
    // 작업 큐에 추가
    m_taskQueue.push(modifiedTask);
    
    return future;
}

ProcessingResult ImageProcessor::executeTask(const ProcessingTask& task) {
    return executeProcessingTask(task);
}

ProcessingResult ImageProcessor::processImage(const std::string& algorithmName, 
                                        const cv::Mat& image,
                                        const std::map<std::string, double>& parameters) {
    ProcessingTask task;
    task.inputImage = image.clone();
    task.algorithmName = algorithmName;
    task.parameters = parameters;
    
    return executeProcessingTask(task);
}

void ImageProcessor::processImageAsync(const std::string& algorithmName,
                                   const cv::Mat& image,
                                   std::function<void(const ProcessingResult&)> callback,
                                   const std::map<std::string, double>& parameters) {
    ProcessingTask task;
    task.inputImage = image.clone();
    task.algorithmName = algorithmName;
    task.parameters = parameters;
    task.callback = callback;
    
    m_taskQueue.push(task);
}

std::vector<std::string> ImageProcessor::getAvailableAlgorithms() const {
    std::lock_guard<std::mutex> lock(m_algorithmsMutex);
    
    std::vector<std::string> algorithms;
    algorithms.reserve(m_algorithms.size());
    
    for (const auto& algorithm : m_algorithms) {
        algorithms.push_back(algorithm->getName());
    }
    
    return algorithms;
}

IProcessingAlgorithm* ImageProcessor::getAlgorithm(const std::string& algorithmName) const {
    return findAlgorithm(algorithmName);
}

bool ImageProcessor::initializeAlgorithms() {
    std::lock_guard<std::mutex> lock(m_algorithmsMutex);
    
    bool allInitialized = true;
    for (const auto& algorithm : m_algorithms) {
        if (!algorithm->initialize()) {
            std::cerr << "Failed to initialize algorithm: " << algorithm->getName() << std::endl;
            allInitialized = false;
        }
    }
    
    return allInitialized;
}

void ImageProcessor::cleanupAlgorithms() {
    std::lock_guard<std::mutex> lock(m_algorithmsMutex);
    
    for (const auto& algorithm : m_algorithms) {
        algorithm->cleanup();
    }
}

// Private 메서드

void ImageProcessor::workerThread() {
    while (!m_shouldStop.load()) {
        ProcessingTask task;
        
        // 작업 대기 (타임아웃으로 종료 신호 확인)
        if (m_taskQueue.waitAndPop(task)) {
            m_activeTasks.fetch_add(1);
            
            // 작업 실행
            ProcessingResult result = executeProcessingTask(task);
            
            // 콜백 호출
            if (task.callback) {
                task.callback(result);
            }
            
            m_activeTasks.fetch_sub(1);
            m_completedTasks.fetch_add(1);
        }
    }
}

ProcessingResult ImageProcessor::executeProcessingTask(const ProcessingTask& task) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    ProcessingResult result;
    result.algorithmName = task.algorithmName;
    
    // 알고리즘 찾기
    IProcessingAlgorithm* algorithm = findAlgorithm(task.algorithmName);
    if (!algorithm) {
        result.success = false;
        result.errorMessage = "Algorithm not found: " + task.algorithmName;
        return result;
    }
    
    // 파라미터 설정
    for (const auto& param : task.parameters) {
        algorithm->setParameter(param.first, param.second);
    }
    
    try {
        // 이미지 처리 가능 여부 확인
        if (!algorithm->canProcess(task.inputImage.type())) {
            result.success = false;
            result.errorMessage = "Algorithm cannot process this image type";
            return result;
        }
        
        // 알고리즘 실행
        result = algorithm->process(task.inputImage);
        result.algorithmName = algorithm->getName();
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = "Processing error: " + std::string(e.what());
        std::cerr << "Algorithm execution error: " << e.what() << std::endl;
        
    } catch (...) {
        result.success = false;
        result.errorMessage = "Unknown processing error";
        std::cerr << "Unknown algorithm execution error" << std::endl;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    return result;
}

IProcessingAlgorithm* ImageProcessor::findAlgorithm(const std::string& algorithmName) const {
    std::lock_guard<std::mutex> lock(m_algorithmsMutex);
    
    auto it = std::find_if(m_algorithms.begin(), m_algorithms.end(),
        [&algorithmName](const std::unique_ptr<IProcessingAlgorithm>& algo) {
            return algo->getName() == algorithmName;
        });
    
    return (it != m_algorithms.end()) ? it->get() : nullptr;
}
