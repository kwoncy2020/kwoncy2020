#pragma once

#include <memory>
#include <string>
#include <opencv2/opencv.hpp>
#include "image/Image.h"
#include "sources/ImageFileSource.h"
#include "sources/VideoFileSource.h"
#include "sources/CameraSource.h"
#include "sources/WindowCaptureSource.h"
#include "processing/ImageProcessor.h"
#include "processing/ProcessingPipeline.h"

/**
 * @brief 데이터 관리자 클래스
 * 
 * 이 클래스는 애플리케이션의 모든 데이터 상태를 중앙에서 관리합니다.
 * 이미지, 데이터 소스, 처리 결과 등을 통합적으로 관리합니다.
 * 
 * 주요 기능:
 * - 이미지 데이터 관리
 * - 데이터 소스 관리
 * - 처리 결과 관리
 * - 애플리케이션 상태 동기화
 */
class DataManager {
public:
    /**
     * @brief 데이터 타입 열거형
     */
    enum class DataType {
        Image,
        Video,
        Camera,
        WindowCapture
    };

private:
    // 현재 데이터
    std::shared_ptr<Image> m_currentImage;               // 현재 이미지
    std::unique_ptr<IDataSource> m_currentDataSource;      // 현재 데이터 소스
    DataType m_currentDataType;                           // 현재 데이터 타입
    
    // 처리 관리
    std::unique_ptr<ImageProcessor> m_imageProcessor;      // 이미지 프로세서
    std::unique_ptr<ProcessingPipeline> m_processingPipeline; // 처리 파이프라인
    std::shared_ptr<Image> m_processedImage;              // 처리된 이미지
    std::vector<std::shared_ptr<Image>> m_processingHistory; // 처리 히스토리
    
    // 데이터 소스들
    std::unique_ptr<ImageFileSource> m_imageFileSource;   // 이미지 파일 소스
    std::unique_ptr<VideoFileSource> m_videoFileSource;   // 비디오 파일 소스
    std::unique_ptr<CameraSource> m_cameraSource;         // 카메라 소스
    std::unique_ptr<WindowCaptureSource> m_windowSource;   // 윈도우 캡처 소스
    
    // 상태 정보
    bool m_hasUnsavedChanges;                             // 저장되지 않은 변경 사항
    std::string m_lastError;                              // 마지막 에러
    std::string m_currentFilePath;                        // 현재 파일 경로
    int m_historyIndex;                                   // 히스토리 인덱스
    
    // 설정 정보
    bool m_autoSave;                                      // 자동 저장 여부
    int m_maxHistorySize;                                 // 최대 히스토리 크기
    bool m_enableProcessingHistory;                        // 처리 히스토리 활성화 여부

public:
    /**
     * @brief 생성자
     */
    DataManager();
    
    /**
     * @brief 소멸자
     */
    ~DataManager();

    // 초기화
    bool initialize();
    void cleanup();
    bool isInitialized() const;

    // 데이터 로딩
    bool loadImage(const std::string& filePath);
    bool loadVideo(const std::string& filePath);
    bool openCamera(int cameraId);
    bool openWindowCapture(const std::string& windowTitle);
    bool openDesktopCapture();
    
    // 데이터 저장
    bool saveCurrentImage(const std::string& filePath);
    bool saveProcessedImage(const std::string& filePath);
    
    // 현재 데이터 접근
    std::shared_ptr<Image> getCurrentImage() const;
    std::shared_ptr<Image> getProcessedImage() const;
    IDataSource* getCurrentDataSource() const;
    DataType getCurrentDataType() const;
    
    // 데이터 설정
    void setCurrentImage(std::shared_ptr<Image> image);
    void setProcessedImage(std::shared_ptr<Image> image);
    
    // 처리 관리
    bool processImage(const std::string& algorithmName);
    bool processWithPipeline();
    void cancelProcessing();
    bool isProcessing() const;
    
    // 파이프라인 관리
    ProcessingPipeline* getProcessingPipeline();
    void resetPipeline();
    
    // 히스토리 관리
    bool canUndo() const;
    bool canRedo() const;
    bool undo();
    bool redo();
    void clearHistory();
    int getHistoryIndex() const;
    
    // 데이터 소스 관리
    ImageFileSource* getImageFileSource();
    VideoFileSource* getVideoFileSource();
    CameraSource* getCameraSource();
    WindowCaptureSource* getWindowCaptureSource();
    
    // 상태 관리
    bool hasUnsavedChanges() const;
    void setUnsavedChanges(bool hasChanges);
    std::string getLastError() const;
    void clearError();
    
    // 파일 경로 관리
    std::string getCurrentFilePath() const;
    void setCurrentFilePath(const std::string& path);
    
    // 설정 관리
    void setAutoSave(bool enabled);
    bool getAutoSave() const;
    void setMaxHistorySize(int size);
    int getMaxHistorySize() const;
    void setProcessingHistoryEnabled(bool enabled);
    bool isProcessingHistoryEnabled() const;
    
    // 데이터 검증
    bool hasCurrentImage() const;
    bool hasProcessedImage() const;
    bool hasValidDataSource() const;
    
    // 데이터 정보
    std::string getCurrentImageInfo() const;
    std::string getDataSourceInfo() const;
    size_t getProcessingHistorySize() const;

private:
    /**
     * @brief 데이터 소스 초기화
     */
    void initializeDataSources();
    
    /**
     * @brief 처리 시스템 초기화
     */
    void initializeProcessingSystem();
    
    /**
     * @brief 이미지 로딩 공통 로직
     * @param dataSource 데이터 소스
     * @param filePath 파일 경로
     * @return 성공 여부
     */
    bool loadImageFromDataSource(IDataSource* dataSource, const std::string& filePath);
    
    /**
     * @brief 처리 히스토리에 추가
     * @param image 처리된 이미지
     */
    void addToProcessingHistory(std::shared_ptr<Image> image);
    
    /**
     * @brief 히스토리 정리 (최대 크기 유지)
     */
    void trimHistory();
    
    /**
     * @brief 에러 설정
     * @param error 에러 메시지
     */
    void setError(const std::string& error);
    
    /**
     * @brief 데이터 타입에 따른 데이터 소스 가져오기
     * @param type 데이터 타입
     * @return 데이터 소스 포인터
     */
    IDataSource* getDataSourceForType(DataType type);
    
    /**
     * @brief 데이터 타입 설정
     * @param type 데이터 타입
     */
    void setDataType(DataType type);
    
    /**
     * @brief 이미지 정보 생성
     * @param image 이미지
     * @return 정보 문자열
     */
    std::string createImageInfo(const std::shared_ptr<Image>& image) const;
    
    /**
     * @brief 데이터 소스 정보 생성
     * @param dataSource 데이터 소스
     * @return 정보 문자열
     */
    std::string createDataSourceInfo(const IDataSource* dataSource) const;
    
    /**
     * @brief 데이터 유효성 검사
     * @param image 검사할 이미지
     * @return 유효성 여부
     */
    bool isImageValid(const std::shared_ptr<Image>& image) const;
    
    /**
     * @brief 데이터 소스 유효성 검사
     * @param dataSource 검사할 데이터 소스
     * @return 유효성 여부
     */
    bool isDataSourceValid(const IDataSource* dataSource) const;
};
