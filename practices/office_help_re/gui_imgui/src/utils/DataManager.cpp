#include "utils/DataManager.h"
#include <iostream>
#include <algorithm>

DataManager::DataManager() 
    : m_currentDataType(DataType::Image), m_hasUnsavedChanges(false),
      m_historyIndex(-1), m_autoSave(false), m_maxHistorySize(50),
      m_enableProcessingHistory(true) {
}

DataManager::~DataManager() {
    cleanup();
}

bool DataManager::initialize() {
    try {
        // 데이터 소스 초기화
        initializeDataSources();
        
        // 처리 시스템 초기화
        initializeProcessingSystem();
        
        std::cout << "DataManager initialized successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        setError("DataManager initialization failed: " + std::string(e.what()));
        std::cerr << "DataManager initialization error: " << e.what() << std::endl;
        return false;
    }
}

void DataManager::cleanup() {
    m_currentImage.reset();
    m_processedImage.reset();
    m_currentDataSource.reset();
    m_imageProcessor.reset();
    m_processingPipeline.reset();
    m_processingHistory.clear();
    
    m_imageFileSource.reset();
    m_videoFileSource.reset();
    m_cameraSource.reset();
    m_windowSource.reset();
    
    std::cout << "DataManager cleaned up" << std::endl;
}

bool DataManager::isInitialized() const {
    return m_imageProcessor && m_processingPipeline && 
           m_imageFileSource && m_videoFileSource && 
           m_cameraSource && m_windowSource;
}

// 데이터 로딩
bool DataManager::loadImage(const std::string& filePath) {
    try {
        if (!m_imageFileSource) {
            setError("Image file source not initialized");
            return false;
        }
        
        if (loadImageFromDataSource(m_imageFileSource.get(), filePath)) {
            setDataType(DataType::Image);
            setCurrentFilePath(filePath);
            clearHistory();
            setUnsavedChanges(false);
            return true;
        }
        
        return false;
        
    } catch (const std::exception& e) {
        setError("Failed to load image: " + std::string(e.what()));
        return false;
    }
}

bool DataManager::loadVideo(const std::string& filePath) {
    try {
        if (!m_videoFileSource) {
            setError("Video file source not initialized");
            return false;
        }
        
        if (m_videoFileSource->open(filePath)) {
            setDataType(DataType::Video);
            setCurrentFilePath(filePath);
            clearHistory();
            setUnsavedChanges(false);
            
            // 첫 프레임 읽기
            cv::Mat frame;
            if (m_videoFileSource->readNextFrame(frame)) {
                m_currentImage = std::make_shared<Image>(frame);
            }
            
            return true;
        }
        
        setError("Failed to open video file: " + m_videoFileSource->getLastError());
        return false;
        
    } catch (const std::exception& e) {
        setError("Failed to load video: " + std::string(e.what()));
        return false;
    }
}

bool DataManager::openCamera(int cameraId) {
    try {
        if (!m_cameraSource) {
            setError("Camera source not initialized");
            return false;
        }
        
        if (m_cameraSource->openCamera(cameraId)) {
            setDataType(DataType::Camera);
            setCurrentFilePath("Camera " + std::to_string(cameraId));
            clearHistory();
            setUnsavedChanges(false);
            
            // 첫 프레임 읽기
            cv::Mat frame;
            if (m_cameraSource->readNextFrame(frame)) {
                m_currentImage = std::make_shared<Image>(frame);
            }
            
            return true;
        }
        
        setError("Failed to open camera: " + m_cameraSource->getLastError());
        return false;
        
    } catch (const std::exception& e) {
        setError("Failed to open camera: " + std::string(e.what()));
        return false;
    }
}

bool DataManager::openWindowCapture(const std::string& windowTitle) {
    try {
        if (!m_windowSource) {
            setError("Window capture source not initialized");
            return false;
        }
        
        if (m_windowSource->openWindow(windowTitle)) {
            setDataType(DataType::WindowCapture);
            setCurrentFilePath("Window: " + windowTitle);
            clearHistory();
            setUnsavedChanges(false);
            
            // 첫 프레임 읽기
            cv::Mat frame;
            if (m_windowSource->readNextFrame(frame)) {
                m_currentImage = std::make_shared<Image>(frame);
            }
            
            return true;
        }
        
        setError("Failed to open window capture: " + m_windowSource->getLastError());
        return false;
        
    } catch (const std::exception& e) {
        setError("Failed to open window capture: " + std::string(e.what()));
        return false;
    }
}

bool DataManager::openDesktopCapture() {
    try {
        if (!m_windowSource) {
            setError("Window capture source not initialized");
            return false;
        }
        
        if (m_windowSource->openDesktop()) {
            setDataType(DataType::WindowCapture);
            setCurrentFilePath("Desktop");
            clearHistory();
            setUnsavedChanges(false);
            
            // 첫 프레임 읽기
            cv::Mat frame;
            if (m_windowSource->readNextFrame(frame)) {
                m_currentImage = std::make_shared<Image>(frame);
            }
            
            return true;
        }
        
        setError("Failed to open desktop capture: " + m_windowSource->getLastError());
        return false;
        
    } catch (const std::exception& e) {
        setError("Failed to open desktop capture: " + std::string(e.what()));
        return false;
    }
}

// 데이터 저장
bool DataManager::saveCurrentImage(const std::string& filePath) {
    try {
        if (!m_currentImage || !isImageValid(m_currentImage)) {
            setError("No valid image to save");
            return false;
        }
        
        cv::Mat mat = m_currentImage->getOpenCVMat();
        if (cv::imwrite(filePath, mat)) {
            setCurrentFilePath(filePath);
            setUnsavedChanges(false);
            return true;
        }
        
        setError("Failed to save image: " + filePath);
        return false;
        
    } catch (const std::exception& e) {
        setError("Failed to save image: " + std::string(e.what()));
        return false;
    }
}

bool DataManager::saveProcessedImage(const std::string& filePath) {
    try {
        if (!m_processedImage || !isImageValid(m_processedImage)) {
            setError("No valid processed image to save");
            return false;
        }
        
        cv::Mat mat = m_processedImage->getOpenCVMat();
        if (cv::imwrite(filePath, mat)) {
            return true;
        }
        
        setError("Failed to save processed image: " + filePath);
        return false;
        
    } catch (const std::exception& e) {
        setError("Failed to save processed image: " + std::string(e.what()));
        return false;
    }
}

// 현재 데이터 접근
std::shared_ptr<Image> DataManager::getCurrentImage() const {
    return m_currentImage;
}

std::shared_ptr<Image> DataManager::getProcessedImage() const {
    return m_processedImage;
}

IDataSource* DataManager::getCurrentDataSource() const {
    return m_currentDataSource.get();
}

DataManager::DataType DataManager::getCurrentDataType() const {
    return m_currentDataType;
}

// 데이터 설정
void DataManager::setCurrentImage(std::shared_ptr<Image> image) {
    if (isImageValid(image)) {
        m_currentImage = image;
        setUnsavedChanges(true);
    }
}

void DataManager::setProcessedImage(std::shared_ptr<Image> image) {
    if (isImageValid(image)) {
        m_processedImage = image;
        setUnsavedChanges(true);
        
        if (m_enableProcessingHistory) {
            addToProcessingHistory(image);
        }
    }
}

// 처리 관리
bool DataManager::processImage(const std::string& algorithmName) {
    try {
        if (!m_currentImage || !isImageValid(m_currentImage)) {
            setError("No valid image to process");
            return false;
        }
        
        if (!m_imageProcessor) {
            setError("Image processor not initialized");
            return false;
        }
        
        // 처리 실행
        ProcessingResult result = m_imageProcessor->process(algorithmName, m_currentImage->getOpenCVMat());
        
        if (result.success) {
            m_processedImage = std::make_shared<Image>(result.processedImage);
            
            if (m_enableProcessingHistory) {
                addToProcessingHistory(m_processedImage);
            }
            
            setUnsavedChanges(true);
            return true;
        }
        
        setError("Processing failed: " + result.errorMessage);
        return false;
        
    } catch (const std::exception& e) {
        setError("Processing error: " + std::string(e.what()));
        return false;
    }
}

bool DataManager::processWithPipeline() {
    try {
        if (!m_currentImage || !isImageValid(m_currentImage)) {
            setError("No valid image to process");
            return false;
        }
        
        if (!m_processingPipeline) {
            setError("Processing pipeline not initialized");
            return false;
        }
        
        // 파이프라인 실행
        std::future<ProcessingResult> future = m_processingPipeline->executeAsync(m_currentImage->getOpenCVMat());
        
        // 결과 대기 (실제 구현에서는 비동기 처리)
        ProcessingResult result = future.get();
        
        if (result.success) {
            m_processedImage = std::make_shared<Image>(result.processedImage);
            
            if (m_enableProcessingHistory) {
                addToProcessingHistory(m_processedImage);
            }
            
            setUnsavedChanges(true);
            return true;
        }
        
        setError("Pipeline processing failed: " + result.errorMessage);
        return false;
        
    } catch (const std::exception& e) {
        setError("Pipeline processing error: " + std::string(e.what()));
        return false;
    }
}

void DataManager::cancelProcessing() {
    if (m_imageProcessor) {
        m_imageProcessor->cancelAllTasks();
    }
    
    if (m_processingPipeline) {
        m_processingPipeline->cancel();
    }
}

bool DataManager::isProcessing() const {
    bool processorActive = m_imageProcessor && m_imageProcessor->isProcessing();
    bool pipelineActive = m_processingPipeline && m_processingPipeline->isProcessing();
    return processorActive || pipelineActive;
}

// 파이프라인 관리
ProcessingPipeline* DataManager::getProcessingPipeline() {
    return m_processingPipeline.get();
}

void DataManager::resetPipeline() {
    if (m_processingPipeline) {
        m_processingPipeline->clear();
    }
}

// 히스토리 관리
bool DataManager::canUndo() const {
    return m_historyIndex > 0;
}

bool DataManager::canRedo() const {
    return m_historyIndex >= 0 && m_historyIndex < static_cast<int>(m_processingHistory.size()) - 1;
}

bool DataManager::undo() {
    if (!canUndo()) {
        return false;
    }
    
    m_historyIndex--;
    if (m_historyIndex >= 0 && m_historyIndex < static_cast<int>(m_processingHistory.size())) {
        m_processedImage = m_processingHistory[m_historyIndex];
        setUnsavedChanges(true);
        return true;
    }
    
    return false;
}

bool DataManager::redo() {
    if (!canRedo()) {
        return false;
    }
    
    m_historyIndex++;
    if (m_historyIndex >= 0 && m_historyIndex < static_cast<int>(m_processingHistory.size())) {
        m_processedImage = m_processingHistory[m_historyIndex];
        setUnsavedChanges(true);
        return true;
    }
    
    return false;
}

void DataManager::clearHistory() {
    m_processingHistory.clear();
    m_historyIndex = -1;
}

int DataManager::getHistoryIndex() const {
    return m_historyIndex;
}

// 데이터 소스 관리
ImageFileSource* DataManager::getImageFileSource() {
    return m_imageFileSource.get();
}

VideoFileSource* DataManager::getVideoFileSource() {
    return m_videoFileSource.get();
}

CameraSource* DataManager::getCameraSource() {
    return m_cameraSource.get();
}

WindowCaptureSource* DataManager::getWindowCaptureSource() {
    return m_windowSource.get();
}

// 상태 관리
bool DataManager::hasUnsavedChanges() const {
    return m_hasUnsavedChanges;
}

void DataManager::setUnsavedChanges(bool hasChanges) {
    m_hasUnsavedChanges = hasChanges;
}

std::string DataManager::getLastError() const {
    return m_lastError;
}

void DataManager::clearError() {
    m_lastError.clear();
}

// 파일 경로 관리
std::string DataManager::getCurrentFilePath() const {
    return m_currentFilePath;
}

void DataManager::setCurrentFilePath(const std::string& path) {
    m_currentFilePath = path;
}

// 설정 관리
void DataManager::setAutoSave(bool enabled) {
    m_autoSave = enabled;
}

bool DataManager::getAutoSave() const {
    return m_autoSave;
}

void DataManager::setMaxHistorySize(int size) {
    m_maxHistorySize = std::max(1, size);
    trimHistory();
}

int DataManager::getMaxHistorySize() const {
    return m_maxHistorySize;
}

void DataManager::setProcessingHistoryEnabled(bool enabled) {
    m_enableProcessingHistory = enabled;
    if (!enabled) {
        clearHistory();
    }
}

bool DataManager::isProcessingHistoryEnabled() const {
    return m_enableProcessingHistory;
}

// 데이터 검증
bool DataManager::hasCurrentImage() const {
    return isImageValid(m_currentImage);
}

bool DataManager::hasProcessedImage() const {
    return isImageValid(m_processedImage);
}

bool DataManager::hasValidDataSource() const {
    return isDataSourceValid(m_currentDataSource.get());
}

// 데이터 정보
std::string DataManager::getCurrentImageInfo() const {
    return createImageInfo(m_currentImage);
}

std::string DataManager::getDataSourceInfo() const {
    return createDataSourceInfo(m_currentDataSource.get());
}

size_t DataManager::getProcessingHistorySize() const {
    return m_processingHistory.size();
}

// Private 메서드

void DataManager::initializeDataSources() {
    m_imageFileSource = std::make_unique<ImageFileSource>();
    m_videoFileSource = std::make_unique<VideoFileSource>();
    m_cameraSource = std::make_unique<CameraSource>();
    m_windowSource = std::make_unique<WindowCaptureSource>();
    
    // 기본 데이터 소스 설정
    m_currentDataSource = std::make_unique<ImageFileSource>();
}

void DataManager::initializeProcessingSystem() {
    m_imageProcessor = std::make_unique<ImageProcessor>();
    m_processingPipeline = std::make_unique<ProcessingPipeline>();
    
    // 기본 알고리즘 등록 (실제로는 외부에서 등록)
    // m_imageProcessor->registerAlgorithm("BoundingBoxDetection", ...);
    // m_imageProcessor->registerAlgorithm("Segmentation", ...);
    // m_imageProcessor->registerAlgorithm("Matching", ...);
}

bool DataManager::loadImageFromDataSource(IDataSource* dataSource, const std::string& filePath) {
    if (!dataSource) {
        setError("Invalid data source");
        return false;
    }
    
    if (!dataSource->open(filePath)) {
        setError("Failed to open data source: " + dataSource->getLastError());
        return false;
    }
    
    cv::Mat frame;
    if (!dataSource->readNextFrame(frame)) {
        setError("Failed to read frame: " + dataSource->getLastError());
        return false;
    }
    
    m_currentImage = std::make_shared<Image>(frame);
    m_currentDataSource.reset(dataSource);
    
    return true;
}

void DataManager::addToProcessingHistory(std::shared_ptr<Image> image) {
    if (!isImageValid(image)) {
        return;
    }
    
    // 현재 인덱스 이후의 히스토리 삭제
    if (m_historyIndex >= 0 && m_historyIndex < static_cast<int>(m_processingHistory.size()) - 1) {
        m_processingHistory.erase(m_processingHistory.begin() + m_historyIndex + 1, m_processingHistory.end());
    }
    
    // 새 이미지 추가
    m_processingHistory.push_back(image);
    m_historyIndex = static_cast<int>(m_processingHistory.size()) - 1;
    
    // 히스토리 크기 제한
    trimHistory();
}

void DataManager::trimHistory() {
    while (m_processingHistory.size() > static_cast<size_t>(m_maxHistorySize)) {
        m_processingHistory.erase(m_processingHistory.begin());
        m_historyIndex--;
    }
    
    // 인덱스 범위 보정
    if (m_historyIndex < -1) {
        m_historyIndex = -1;
    }
    if (m_historyIndex >= static_cast<int>(m_processingHistory.size())) {
        m_historyIndex = static_cast<int>(m_processingHistory.size()) - 1;
    }
}

void DataManager::setError(const std::string& error) {
    m_lastError = error;
    std::cerr << "DataManager Error: " << error << std::endl;
}

IDataSource* DataManager::getDataSourceForType(DataType type) {
    switch (type) {
        case DataType::Image:
            return m_imageFileSource.get();
        case DataType::Video:
            return m_videoFileSource.get();
        case DataType::Camera:
            return m_cameraSource.get();
        case DataType::WindowCapture:
            return m_windowSource.get();
        default:
            return nullptr;
    }
}

void DataManager::setDataType(DataType type) {
    m_currentDataType = type;
    m_currentDataSource.reset(getDataSourceForType(type));
}

std::string DataManager::createImageInfo(const std::shared_ptr<Image>& image) const {
    if (!isImageValid(image)) {
        return "No image";
    }
    
    cv::Mat mat = image->getOpenCVMat();
    std::ostringstream info;
    info << mat.cols << "x" << mat.rows << " (" << mat.channels() << " channels)";
    return info.str();
}

std::string DataManager::createDataSourceInfo(const IDataSource* dataSource) const {
    if (!isDataSourceValid(dataSource)) {
        return "No data source";
    }
    
    DataSourceInfo info = dataSource->getSourceInfo();
    return info.sourceType + ": " + info.sourcePath;
}

bool DataManager::isImageValid(const std::shared_ptr<Image>& image) const {
    return image && !image->getOpenCVMat().empty();
}

bool DataManager::isDataSourceValid(const IDataSource* dataSource) const {
    return dataSource && dataSource->isOpen();
}
