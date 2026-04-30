#include "ui/ControlPanel.h"
#include "imgui.h"
#include <cstring>
#include <sstream>

ControlPanel::ControlPanel() 
    : m_showImageInfo(true), m_autoFitToWindow(true),
      m_zoomLevel(1.0f), m_showPixelInfo(true), m_showGrid(false),
      m_showAnnotations(true), m_selectedAlgorithm(0), m_isProcessing(false),
      m_processingProgress(0.0f), m_selectedDataSource(0),
      m_isPlaying(false), m_playbackSpeed(1.0) {
    
    // 초기값 설정
    strcpy_s(m_filePath, sizeof(m_filePath), "");
    strcpy_s(m_cameraId, sizeof(m_cameraId), "0");
    strcpy_s(m_videoPath, sizeof(m_videoPath), "");
}

void ControlPanel::render() {
    ImGui::Begin("Control Panel");
    
    // 이미지 컨트롤
    renderImageControls();
    ImGui::Separator();
    
    // 뷰어 컨트롤
    renderViewerControls();
    ImGui::Separator();
    
    // 데이터 소스 컨트롤
    renderDataSourceControls();
    ImGui::Separator();
    
    // 처리 컨트롤
    renderProcessingControls();
    
    ImGui::End();
}

// 이미지 컨트롤 콜백 설정
void ControlPanel::setLoadImageCallback(StringCallback callback) {
    m_loadImageCallback = callback;
}

void ControlPanel::setShowImageInfoCallback(BoolCallback callback) {
    // 이 콜백은 내부 상태 변경에 사용
}

void ControlPanel::setAutoFitToWindowCallback(BoolCallback callback) {
    m_autoFitToWindowCallback = callback;
}

// 뷰어 컨트롤 콜백 설정
void ControlPanel::setZoomCallback(FloatCallback callback) {
    m_zoomCallback = callback;
}

void ControlPanel::setShowPixelInfoCallback(BoolCallback callback) {
    // 이 콜백은 내부 상태 변경에 사용
}

void ControlPanel::setShowGridCallback(BoolCallback callback) {
    // 이 콜백은 내부 상태 변경에 사용
}

void ControlPanel::setShowAnnotationsCallback(BoolCallback callback) {
    // 이 콜백은 내부 상태 변경에 사용
}

// 처리 컨트롤 콜백 설정
void ControlPanel::setProcessAlgorithmCallback(IntCallback callback) {
    m_processAlgorithmCallback = callback;
}

void ControlPanel::setParameterChangedCallback(FloatCallback callback) {
    m_parameterChangedCallback = callback;
}

// 데이터 소스 콜백 설정
void ControlPanel::setLoadVideoCallback(StringCallback callback) {
    m_loadVideoCallback = callback;
}

void ControlPanel::setOpenCameraCallback(IntCallback callback) {
    m_openCameraCallback = callback;
}

void ControlPanel::setOpenWindowCallback(StringCallback callback) {
    m_openWindowCallback = callback;
}

void ControlPanel::setPlaybackControlCallback(BoolCallback callback) {
    // 이 콜백은 내부 상태 변경에 사용
}

// 상태 설정 메서드들
void ControlPanel::setImagePath(const std::string& path) {
    strcpy_s(m_filePath, sizeof(m_filePath), path.c_str());
}

void ControlPanel::setImageInfo(const std::string& width, const std::string& height, const std::string& channels) {
    // 이미지 정보 업데이트 (내부적으로 처리)
}

void ControlPanel::setZoomLevel(float zoom) {
    m_zoomLevel = zoom;
}

void ControlPanel::setProcessingState(bool isProcessing, float progress) {
    m_isProcessing = isProcessing;
    m_processingProgress = progress;
}

void ControlPanel::setPlayingState(bool isPlaying) {
    m_isPlaying = isPlaying;
}

void ControlPanel::setPlaybackSpeed(double speed) {
    m_playbackSpeed = speed;
}

// 파라미터 설정 메서드들
void ControlPanel::setConfidenceThreshold(double threshold) {
    m_params.confidenceThreshold = threshold;
}

void ControlPanel::setNmsThreshold(double threshold) {
    m_params.nmsThreshold = threshold;
}

void ControlPanel::setMinObjectSize(int width, int height) {
    m_params.minObjectWidth = width;
    m_params.minObjectHeight = height;
}

void ControlPanel::setNumClasses(int numClasses) {
    m_params.numClasses = numClasses;
}

void ControlPanel::setThresholdValue(double value) {
    m_params.thresholdValue = value;
}

void ControlPanel::setMatchThreshold(double threshold) {
    m_params.matchThreshold = threshold;
}

// 상태 조회 메서드들
std::string ControlPanel::getFilePath() const {
    return std::string(m_filePath);
}

float ControlPanel::getZoomLevel() const {
    return m_zoomLevel;
}

int ControlPanel::getSelectedAlgorithm() const {
    return m_selectedAlgorithm;
}

int ControlPanel::getSelectedDataSource() const {
    return m_selectedDataSource;
}

bool ControlPanel::isProcessing() const {
    return m_isProcessing;
}

bool ControlPanel::isPlaying() const {
    return m_isPlaying;
}

// 파라미터 조회 메서드들
double ControlPanel::getConfidenceThreshold() const {
    return m_params.confidenceThreshold;
}

double ControlPanel::getNmsThreshold() const {
    return m_params.nmsThreshold;
}

int ControlPanel::getMinObjectWidth() const {
    return m_params.minObjectWidth;
}

int ControlPanel::getMinObjectHeight() const {
    return m_params.minObjectHeight;
}

int ControlPanel::getNumClasses() const {
    return m_params.numClasses;
}

double ControlPanel::getThresholdValue() const {
    return m_params.thresholdValue;
}

double ControlPanel::getMatchThreshold() const {
    return m_params.matchThreshold;
}

// Private 메서드

void ControlPanel::renderImageControls() {
    if (ImGui::CollapsingHeader("Image Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 파일 경로 입력
        if (ImGui::InputText("File Path", m_filePath, sizeof(m_filePath))) {
            // 파일 경로 변경 처리
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Browse")) {
            std::string filePath = showFileDialog("Open Image File", "Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tiff)\0*.png;*.jpg;*.jpeg;*.bmp;*.tiff\0All Files (*.*)\0*.*\0");
            if (!filePath.empty()) {
                strcpy_s(m_filePath, sizeof(m_filePath), filePath.c_str());
                if (m_loadImageCallback) {
                    m_loadImageCallback(filePath);
                }
            }
        }
        
        // 이미지 정보 표시
        if (ImGui::Checkbox("Show Image Info", &m_showImageInfo)) {
            // 이미지 정보 표시 상태 변경
        }
        
        // 자동 창 맞춤
        if (ImGui::Checkbox("Auto Fit to Window", &m_autoFitToWindow)) {
            if (m_autoFitToWindowCallback) {
                m_autoFitToWindowCallback(m_autoFitToWindow);
            }
        }
    }
}

void ControlPanel::renderViewerControls() {
    if (ImGui::CollapsingHeader("Viewer Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 확대/축소 슬라이더
        if (ImGui::SliderFloat("Zoom", &m_zoomLevel, 0.1f, 10.0f, "%.2fx")) {
            if (m_zoomCallback) {
                m_zoomCallback(m_zoomLevel);
            }
        }
        
        // 확대/축소 버튼들
        if (ImGui::Button("Zoom In")) {
            m_zoomLevel = std::min(m_zoomLevel * 1.2f, 10.0f);
            if (m_zoomCallback) {
                m_zoomCallback(m_zoomLevel);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Zoom Out")) {
            m_zoomLevel = std::max(m_zoomLevel / 1.2f, 0.1f);
            if (m_zoomCallback) {
                m_zoomCallback(m_zoomLevel);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            m_zoomLevel = 1.0f;
            if (m_zoomCallback) {
                m_zoomCallback(m_zoomLevel);
            }
        }
        
        // 표시 옵션들
        if (ImGui::Checkbox("Show Pixel Info", &m_showPixelInfo)) {
            // 픽셀 정보 표시 상태 변경
        }
        
        if (ImGui::Checkbox("Show Grid", &m_showGrid)) {
            // 그리드 표시 상태 변경
        }
        
        if (ImGui::Checkbox("Show Annotations", &m_showAnnotations)) {
            // 주석 표시 상태 변경
        }
    }
}

void ControlPanel::renderDataSourceControls() {
    if (ImGui::CollapsingHeader("Data Source", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 데이터 소스 선택
        const char* dataSourceItems[] = {"Image File", "Video File", "Camera", "Window Capture"};
        if (ImGui::Combo("Source Type", &m_selectedDataSource, dataSourceItems, IM_ARRAYSIZE(dataSourceItems))) {
            // 데이터 소스 타입 변경 처리
        }
        
        // 데이터 소스별 컨트롤
        switch (m_selectedDataSource) {
            case 0: // 이미지 파일
                // 이미지 파일 컨트롤은 이미지 컨트롤 섹션에서 처리
                break;
                
            case 1: // 비디오 파일
                if (ImGui::InputText("Video Path", m_videoPath, sizeof(m_videoPath))) {
                    // 비디오 경로 변경 처리
                }
                ImGui::SameLine();
                if (ImGui::Button("Browse Video")) {
                    std::string videoPath = showFileDialog("Open Video File", "Video Files (*.mp4;*.avi;*.mov;*.mkv)\0*.mp4;*.avi;*.mov;*.mkv\0All Files (*.*)\0*.*\0");
                    if (!videoPath.empty()) {
                        strcpy_s(m_videoPath, sizeof(m_videoPath), videoPath.c_str());
                        if (m_loadVideoCallback) {
                            m_loadVideoCallback(videoPath);
                        }
                    }
                }
                
                // 재생 컨트롤
                if (ImGui::Button(m_isPlaying ? "Pause" : "Play")) {
                    m_isPlaying = !m_isPlaying;
                    if (m_playbackControlCallback) {
                        m_playbackControlCallback(m_isPlaying);
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Stop")) {
                    m_isPlaying = false;
                    if (m_playbackControlCallback) {
                        m_playbackControlCallback(m_isPlaying);
                    }
                }
                
                if (ImGui::SliderFloat("Playback Speed", &m_playbackSpeed, 0.1f, 4.0f, "%.1fx")) {
                    // 재생 속도 변경 처리
                }
                break;
                
            case 2: // 카메라
                if (ImGui::InputText("Camera ID", m_cameraId, sizeof(m_cameraId))) {
                    // 카메라 ID 변경 처리
                }
                ImGui::SameLine();
                if (ImGui::Button("Open Camera")) {
                    try {
                        int cameraId = std::stoi(m_cameraId);
                        if (m_openCameraCallback) {
                            m_openCameraCallback(cameraId);
                        }
                    } catch (const std::exception&) {
                        // 잘못된 카메라 ID 처리
                    }
                }
                break;
                
            case 3: // 윈도우 캡처
                if (ImGui::Button("Capture Desktop")) {
                    if (m_openWindowCallback) {
                        m_openWindowCallback("desktop");
                    }
                }
                if (ImGui::Button("Capture Active Window")) {
                    if (m_openWindowCallback) {
                        m_openWindowCallback("active");
                    }
                }
                break;
        }
    }
}

void ControlPanel::renderProcessingControls() {
    if (ImGui::CollapsingHeader("Processing", ImGuiTreeNodeFlags_DefaultOpen)) {
        // 알고리즘 선택
        const char* algorithmItems[] = {"Bounding Box Detection", "Segmentation", "Image Matching"};
        if (ImGui::Combo("Algorithm", &m_selectedAlgorithm, algorithmItems, IM_ARRAYSIZE(algorithmItems))) {
            if (m_processAlgorithmCallback) {
                m_processAlgorithmCallback(m_selectedAlgorithm);
            }
        }
        
        // 처리 버튼
        if (m_isProcessing) {
            // 진행률 표시
            ImGui::ProgressBar(m_processingProgress / 100.0f, ImVec2(-1, 0), nullptr);
            ImGui::SameLine();
            ImGui::Text("%.0f%%", m_processingProgress);
            
            if (ImGui::Button("Cancel")) {
                // 처리 취소 로직
                m_isProcessing = false;
                m_processingProgress = 0.0f;
            }
        } else {
            if (ImGui::Button("Process")) {
                m_isProcessing = true;
                m_processingProgress = 0.0f;
                if (m_processAlgorithmCallback) {
                    m_processAlgorithmCallback(m_selectedAlgorithm);
                }
            }
        }
        
        // 알고리즘 파라미터
        renderAlgorithmParameters();
    }
}

void ControlPanel::renderAlgorithmParameters() {
    if (ImGui::CollapsingHeader("Algorithm Parameters")) {
        switch (m_selectedAlgorithm) {
            case 0: // Bounding Box Detection
                renderBoundingBoxParams();
                break;
            case 1: // Segmentation
                renderSegmentationParams();
                break;
            case 2: // Image Matching
                renderMatchingParams();
                break;
        }
    }
}

void ControlPanel::renderBoundingBoxParams() {
    // 신뢰도 임계값
    float confidence = static_cast<float>(m_params.confidenceThreshold);
    if (ImGui::SliderFloat("Confidence Threshold", &confidence, 0.0f, 1.0f, "%.2f")) {
        m_params.confidenceThreshold = confidence;
        if (m_parameterChangedCallback) {
            m_parameterChangedCallback(confidence);
        }
    }
    
    // NMS 임계값
    float nmsThreshold = static_cast<float>(m_params.nmsThreshold);
    if (ImGui::SliderFloat("NMS Threshold", &nmsThreshold, 0.0f, 1.0f, "%.2f")) {
        m_params.nmsThreshold = nmsThreshold;
        if (m_parameterChangedCallback) {
            m_parameterChangedCallback(nmsThreshold);
        }
    }
    
    // 최소 객체 크기
    int minWidth = m_params.minObjectWidth;
    int minHeight = m_params.minObjectHeight;
    if (ImGui::InputInt("Min Width", &minWidth)) {
        m_params.minObjectWidth = std::max(1, minWidth);
    }
    if (ImGui::InputInt("Min Height", &minHeight)) {
        m_params.minObjectHeight = std::max(1, minHeight);
    }
    
    // 멀티스케일 사용
    if (ImGui::Checkbox("Use Multi-Scale", &m_params.useMultiScale)) {
        // 멀티스케일 옵션 변경 처리
    }
}

void ControlPanel::renderSegmentationParams() {
    // 클래스 수
    if (ImGui::InputInt("Number of Classes", &m_params.numClasses)) {
        m_params.numClasses = std::max(2, m_params.numClasses);
    }
    
    // 임계값
    float threshold = static_cast<float>(m_params.thresholdValue);
    if (ImGui::SliderFloat("Threshold", &threshold, 0.0f, 255.0f, "%.1f")) {
        m_params.thresholdValue = threshold;
        if (m_parameterChangedCallback) {
            m_parameterChangedCallback(threshold);
        }
    }
    
    // 세그멘테이션 방법
    const char* methodItems[] = {"Thresholding", "Otsu", "Adaptive", "Watershed", "K-Means"};
    if (ImGui::Combo("Method", &m_params.segmentationMethod, methodItems, IM_ARRAYSIZE(methodItems))) {
        // 세그멘테이션 방법 변경 처리
    }
    
    // 형태학적 연산 사용
    if (ImGui::Checkbox("Use Morphology", &m_params.useMorphology)) {
        // 형태학적 연산 옵션 변경 처리
    }
}

void ControlPanel::renderMatchingParams() {
    // 매칭 임계값
    float matchThreshold = static_cast<float>(m_params.matchThreshold);
    if (ImGui::SliderFloat("Match Threshold", &matchThreshold, 0.0f, 1.0f, "%.2f")) {
        m_params.matchThreshold = matchThreshold;
        if (m_parameterChangedCallback) {
            m_parameterChangedCallback(matchThreshold);
        }
    }
    
    // 템플릿 매칭 방법
    const char* templateMethods[] = {"TM_CCOEFF_NORMED", "TM_SQDIFF_NORMED", "TM_CCORR_NORMED"};
    if (ImGui::Combo("Template Method", &m_params.templateMatchingMethod, templateMethods, IM_ARRAYSIZE(templateMethods))) {
        // 템플릿 매칭 방법 변경 처리
    }
    
    // 회전 불변성
    if (ImGui::Checkbox("Rotation Invariant", &m_params.useRotationInvariant)) {
        // 회전 불변성 옵션 변경 처리
    }
    
    // 스케일 범위
    float scaleMin = static_cast<float>(m_params.scaleRangeMin);
    float scaleMax = static_cast<float>(m_params.scaleRangeMax);
    if (ImGui::SliderFloat("Scale Min", &scaleMin, 0.1f, 2.0f, "%.1f")) {
        m_params.scaleRangeMin = scaleMin;
    }
    if (ImGui::SliderFloat("Scale Max", &scaleMax, 0.1f, 5.0f, "%.1f")) {
        m_params.scaleRangeMax = scaleMax;
    }
}

std::string ControlPanel::showFileDialog(const char* title, const char* filter) {
    // 실제 파일 다이얼로그 구현은 플랫폼 종속적
    // 여기서는 간단한 구현으로 대체
    return "";
}

const char* ControlPanel::getAlgorithmName(int index) const {
    const char* names[] = {"Bounding Box Detection", "Segmentation", "Image Matching"};
    if (index >= 0 && index < IM_ARRAYSIZE(names)) {
        return names[index];
    }
    return "Unknown";
}

const char* ControlPanel::getDataSourceName(int index) const {
    const char* names[] = {"Image File", "Video File", "Camera", "Window Capture"};
    if (index >= 0 && index < IM_ARRAYSIZE(names)) {
        return names[index];
    }
    return "Unknown";
}

std::string ControlPanel::formatImageInfo(int width, int height, int channels) const {
    std::ostringstream oss;
    oss << width << "x" << height << " (" << channels << " channels)";
    return oss.str();
}
