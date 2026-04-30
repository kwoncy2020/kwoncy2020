#pragma once
 
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <opencv2/opencv.hpp>

/**
 * @brief 컨트롤 패널 UI 클래스
 * 
 * 이 클래스는 애플리케이션의 주요 컨트롤들을 관리합니다.
 * 이미지 로딩, 뷰어 제어, 처리 알고리즘 설정 등을 제공합니다.
 * 
 * 주요 기능:
 * - 이미지 파일 로딩
 * - 뷰어 확대/축소 및 패닝 제어
 * - 처리 알고리즘 파라미터 설정
 * - 데이터 소스 선택 및 제어
 */
class ControlPanel {
public:
    /**
     * @brief 컨트롤 이벤트 콜백 타입
     */
    using StringCallback = std::function<void(const std::string&)>;
    using BoolCallback = std::function<void(bool)>;
    using FloatCallback = std::function<void(float)>;
    using IntCallback = std::function<void(int)>;

private:
    // 이미지 컨트롤
    char m_filePath[256];                                   // 파일 경로 버퍼
    bool m_showImageInfo;                                   // 이미지 정보 표시 여부
    bool m_autoFitToWindow;                                 // 자동 창 맞춤 여부
    
    // 뷰어 컨트롤
    float m_zoomLevel;                                      // 확대/축소 레벨
    bool m_showPixelInfo;                                   // 픽셀 정보 표시 여부
    bool m_showGrid;                                        // 그리드 표시 여부
    bool m_showAnnotations;                                 // 주석 표시 여부
    
    // 처리 컨트롤
    int m_selectedAlgorithm;                                // 선택된 알고리즘
    bool m_isProcessing;                                    // 처리 중 상태
    float m_processingProgress;                             // 처리 진행률
    
    // 알고리즘 파라미터들
    struct AlgorithmParams {
        // Bounding Box Detection
        double confidenceThreshold;
        double nmsThreshold;
        int minObjectWidth;
        int minObjectHeight;
        bool useMultiScale;
        
        // Segmentation
        int numClasses;
        double thresholdValue;
        int segmentationMethod;
        bool useMorphology;
        
        // Image Matching
        double matchThreshold;
        int templateMatchingMethod;
        bool useRotationInvariant;
        double scaleRangeMin;
        double scaleRangeMax;
        
        AlgorithmParams() 
            : confidenceThreshold(0.5), nmsThreshold(0.4), 
              minObjectWidth(10), minObjectHeight(10), useMultiScale(true),
              numClasses(2), thresholdValue(128.0), segmentationMethod(1),
              useMorphology(true), matchThreshold(0.7), templateMatchingMethod(0),
              useRotationInvariant(false), scaleRangeMin(0.5), scaleRangeMax(2.0) {}
    } m_params;
    
    // 데이터 소스 컨트롤
    int m_selectedDataSource;                              // 선택된 데이터 소스
    char m_cameraId[16];                                    // 카메라 ID 버퍼
    char m_videoPath[256];                                  // 비디오 파일 경로 버퍼
    bool m_isPlaying;                                       // 재생 상태
    double m_playbackSpeed;                                 // 재생 속도
    
    // 콜백 함수들
    StringCallback m_loadImageCallback;
    StringCallback m_loadVideoCallback;
    IntCallback m_openCameraCallback;
    StringCallback m_openWindowCallback;
    FloatCallback m_zoomCallback;
    BoolCallback m_fitToWindowCallback;
    IntCallback m_processAlgorithmCallback;
    FloatCallback m_parameterChangedCallback;

public:
    /**
     * @brief 생성자
     */
    ControlPanel();
    
    /**
     * @brief 소멸자
     */
    ~ControlPanel() = default;

    // 패널 렌더링
    void render();

    // 이미지 컨트롤 콜백 설정
    void setLoadImageCallback(StringCallback callback);
    void setShowImageInfoCallback(BoolCallback callback);
    void setAutoFitToWindowCallback(BoolCallback callback);
    
    // 뷰어 컨트롤 콜백 설정
    void setZoomCallback(FloatCallback callback);
    void setShowPixelInfoCallback(BoolCallback callback);
    void setShowGridCallback(BoolCallback callback);
    void setShowAnnotationsCallback(BoolCallback callback);
    
    // 처리 컨트롤 콜백 설정
    void setProcessAlgorithmCallback(IntCallback callback);
    void setParameterChangedCallback(FloatCallback callback);
    
    // 데이터 소스 콜백 설정
    void setLoadVideoCallback(StringCallback callback);
    void setOpenCameraCallback(IntCallback callback);
    void setOpenWindowCallback(StringCallback callback);
    void setPlaybackControlCallback(BoolCallback callback);

    // 상태 설정 메서드들
    void setImagePath(const std::string& path);
    void setImageInfo(const std::string& width, const std::string& height, const std::string& channels);
    void setZoomLevel(float zoom);
    void setProcessingState(bool isProcessing, float progress = 0.0f);
    void setPlayingState(bool isPlaying);
    void setPlaybackSpeed(double speed);

    // 파라미터 설정 메서드들
    void setConfidenceThreshold(double threshold);
    void setNmsThreshold(double threshold);
    void setMinObjectSize(int width, int height);
    void setNumClasses(int numClasses);
    void setThresholdValue(double value);
    void setMatchThreshold(double threshold);

    // 상태 조회 메서드들
    std::string getFilePath() const;
    float getZoomLevel() const;
    int getSelectedAlgorithm() const;
    int getSelectedDataSource() const;
    bool isProcessing() const;
    bool isPlaying() const;
    
    // 파라미터 조회 메서드들
    double getConfidenceThreshold() const;
    double getNmsThreshold() const;
    int getMinObjectWidth() const;
    int getMinObjectHeight() const;
    int getNumClasses() const;
    double getThresholdValue() const;
    double getMatchThreshold() const;

private:
    /**
     * @brief 이미지 컨트롤 섹션 렌더링
     */
    void renderImageControls();
    
    /**
     * @brief 뷰어 컨트롤 섹션 렌더링
     */
    void renderViewerControls();
    
    /**
     * @brief 처리 컨트롤 섹션 렌더링
     */
    void renderProcessingControls();
    
    /**
     * @brief 데이터 소스 컨트롤 섹션 렌더링
     */
    void renderDataSourceControls();
    
    /**
     * @brief 알고리즘 파라미터 섹션 렌더링
     */
    void renderAlgorithmParameters();
    
    /**
     * @brief Bounding Box Detection 파라미터 렌더링
     */
    void renderBoundingBoxParams();
    
    /**
     * @brief Segmentation 파라미터 렌더링
     */
    void renderSegmentationParams();
    
    /**
     * @brief Image Matching 파라미터 렌더링
     */
    void renderMatchingParams();
    
    /**
     * @brief 파일 다이얼로그 표시
     * @param title 다이얼로그 제목
     * @param filter 파일 필터
     * @return 선택된 파일 경로
     */
    std::string showFileDialog(const char* title, const char* filter);
    
    /**
     * @brief 알고리즘 이름 가져오기
     * @param index 알고리즘 인덱스
     * @return 알고리즘 이름
     */
    const char* getAlgorithmName(int index) const;
    
    /**
     * @brief 데이터 소스 이름 가져오기
     * @param index 데이터 소스 인덱스
     * @return 데이터 소스 이름
     */
    const char* getDataSourceName(int index) const;
    
    /**
     * @brief 이미지 정보 포맷팅
     * @param width 이미지 너비
     * @param height 이미지 높이
     * @param channels 채널 수
     * @return 포맷된 정보 문자열
     */
    std::string formatImageInfo(int width, int height, int channels) const;
};
