/**
 * @file main.cpp
 * @brief 백엔드 중립적 이미지 처리 애플리케이션 메인
 * 
 * 완전한 아키텍처를 사용한 엔터프라이즈급 이미지 처리 애플리케이션입니다.
 * 모듈화된 UI 패널, 데이터 관리자, 영상처리 엔진을 통합합니다.
 */

#include "utils/DataManager.h"
#include "ui/MenuBar.h"
#include "ui/ControlPanel.h"
#include "ui/StatusBar.h"
#include "graphics/BackendFactory.h"
#include "image/ImageViewer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <string>
#include <chrono>

/**
 * @brief 메인 애플리케이션 클래스
 * 
 * 모든 컴포넌트를 통합하여 전체 애플리케이션 로직을 관리합니다.
 */
class ImageProcessingApp {
private:
    // 핵심 컴포넌트
    std::unique_ptr<DataManager> m_dataManager;              // 데이터 관리자
    std::unique_ptr<IGraphicsBackend> m_graphicsBackend;      // 그래픽 백엔드
    
    // UI 패널들
    std::unique_ptr<MenuBar> m_menuBar;                     // 메뉴 바
    std::unique_ptr<ControlPanel> m_controlPanel;           // 컨트롤 패널
    std::unique_ptr<StatusBar> m_statusBar;                 // 상태 바
    
    // 이미지 뷰어
    std::unique_ptr<ImageViewer> m_imageViewer;             // 이미지 뷰어
    
    // 애플리케이션 상태
    bool m_initialized = false;                            // 초기화 상태
    bool m_running = true;                                 // 실행 상태
    std::string m_lastError;                               // 마지막 에러
    
    // 성능 모니터링
    std::chrono::steady_clock::time_point m_frameStartTime;   // 프레임 시작 시간
    double m_currentFPS = 0.0;                           // 현재 FPS
    int m_frameCount = 0;                                 // 프레임 카운트

public:
    /**
     * @brief 생성자
     */
    ImageProcessingApp() {
        initialize();
    }
    
    /**
     * @brief 소멸자
     */
    ~ImageProcessingApp() {
        cleanup();
    }
    
    /**
     * @brief 애플리케이션 초기화
     * @return 성공 여부
     */
    bool initialize() {
        if (m_initialized) {
            return true;
        }
        
        try {
            // 데이터 관리자 초기화
            m_dataManager = std::make_unique<DataManager>();
            if (!m_dataManager->initialize()) {
                m_lastError = "Failed to initialize DataManager: " + m_dataManager->getLastError();
                return false;
            }
            
            // 그래픽 백엔드 생성
            m_graphicsBackend = BackendFactory::createRecommendedBackend();
            if (!m_graphicsBackend) {
                m_lastError = "Failed to create graphics backend";
                return false;
            }
            
            // 그래픽 백엔드 초기화
            if (!m_graphicsBackend->initialize()) {
                m_lastError = "Failed to initialize graphics backend";
                return false;
            }
            
            // UI 패널들 생성
            m_menuBar = std::make_unique<MenuBar>();
            m_controlPanel = std::make_unique<ControlPanel>();
            m_statusBar = std::make_unique<StatusBar>();
            
            // 이미지 뷰어 생성
            m_imageViewer = std::make_unique<ImageViewer>("Main Image Viewer");
            
            // 콜백 설정
            setupCallbacks();
            
            // 성능 모니터링 초기화
            m_frameStartTime = std::chrono::steady_clock::now();
            
            m_initialized = true;
            std::cout << "Application shutdown complete" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            m_lastError = "Initialization failed: " + std::string(e.what());
            std::cerr << m_lastError << std::endl;
            return false;
        }
    }
    
    /**
     * @brief 실행 상태 확인
     * @return 실행 여부
     */
    bool isRunning() const {
        return m_running && m_graphicsBackend && !m_graphicsBackend->shouldClose();
    }
    
    /**
     * @brief 메인 렌더링 함수
     */
    void render() {
        if (!m_initialized) {
            renderErrorScreen();
            return;
        }
        
        // 성능 모니터링 시작
        auto frameStart = std::chrono::steady_clock::now();
        
        // 메뉴 바 렌더링
        m_menuBar->render();
        
        // 메인 렌더링 영역
        renderMainContent();
        
        // 컨트롤 패널 렌더링
        m_controlPanel->render();
        
        // 상태 바 렌더링
        m_statusBar->render();
        
        // 상태 업데이트
        updateUIState();
        updatePerformanceMetrics();
    }
    
    /**
     * @brief 초기화 상태 확인
     * @return 초기화 완료 여부
     */
    bool isInitialized() const {
        return m_initialized;
    }
    
    /**
     * @brief 애플리케이션 종료 요청
     */
    void requestExit() {
        m_running = false;
    }
    
    /**
     * @brief 마지막 에러 메시지 가져오기
     * @return 에러 메시지
     */
    const std::string& getLastError() const {
        return m_lastError;
    }

private:
    /**
     * @brief 콜백 설정
     */
    void setupCallbacks() {
        if (!m_menuBar || !m_controlPanel || !m_statusBar || !m_dataManager) {
            return;
        }
        
        // 메뉴 바 콜백
        m_menuBar->setOpenFileCallback([this](const std::string& path) {
            if (!path.empty()) {
                m_dataManager->loadImage(path);
            }
        });
        
        m_menuBar->setSaveFileCallback([this]() {
            auto currentPath = m_dataManager->getCurrentFilePath();
            if (!currentPath.empty()) {
                m_dataManager->saveCurrentImage(currentPath);
            }
        });
        
        m_menuBar->setSaveAsFileCallback([this](const std::string& path) {
            if (!path.empty()) {
                m_dataManager->saveCurrentImage(path);
            }
        });
        
        m_menuBar->setBoundingBoxDetectionCallback([this]() {
            m_dataManager->processImage("BoundingBoxDetection");
        });
        
        m_menuBar->setSegmentationCallback([this]() {
            m_dataManager->processImage("Segmentation");
        });
        
        m_menuBar->setMatchingCallback([this]() {
            m_dataManager->processImage("Matching");
        });
        
        m_menuBar->setUndoCallback([this]() {
            if (m_dataManager->canUndo()) {
                m_dataManager->undo();
            }
        });
        
        m_menuBar->setRedoCallback([this]() {
            if (m_dataManager->canRedo()) {
                m_dataManager->redo();
            }
        });
        
        m_menuBar->setExitCallback([this]() {
            requestExit();
        });
        
        // 컨트롤 패널 콜백
        m_controlPanel->setLoadImageCallback([this](const std::string& path) {
            m_dataManager->loadImage(path);
        });
        
        m_controlPanel->setLoadVideoCallback([this](const std::string& path) {
            m_dataManager->loadVideo(path);
        });
        
        m_controlPanel->setOpenCameraCallback([this](int cameraId) {
            m_dataManager->openCamera(cameraId);
        });
        
        m_controlPanel->setOpenWindowCaptureCallback([this](const std::string& windowTitle) {
            m_dataManager->openWindowCapture(windowTitle);
        });
        
        m_controlPanel->setProcessAlgorithmCallback([this](int algorithm) {
            const char* algorithms[] = {"BoundingBoxDetection", "Segmentation", "Matching"};
            if (algorithm >= 0 && algorithm < 3) {
                m_dataManager->processImage(algorithms[algorithm]);
            }
        });
        
        m_controlPanel->setZoomCallback([this](float zoom) {
            // 뷰어 줌 레벨 업데이트 (실제 뷰어가 구현되면 연결)
            std::cout << "Zoom level: " << zoom << std::endl;
        });
        
        m_controlPanel->setSaveProcessedImageCallback([this](const std::string& path) {
            m_dataManager->saveProcessedImage(path);
        });
    }
    
    /**
     * @brief 성능 메트릭 업데이트
     */
    void updatePerformanceMetrics() {
        auto frameEnd = std::chrono::steady_clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - m_frameStartTime);
        
        m_frameCount++;
        
        // 1초마다 FPS 업데이트
        if (frameDuration.count() >= 1000) {
            m_currentFPS = static_cast<double>(m_frameCount) * 1000.0 / frameDuration.count();
            m_frameCount = 0;
            m_frameStartTime = frameEnd;
        }
    }
    
    /**
     * @brief 메인 콘텐츠 렌더링
     */
    void renderMainContent() {
        // 메인 뷰포트 창
        ImGui::Begin("Main Viewport", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        
        // 이미지 뷰어 영역 - 실제 ImageViewer 연동
        auto currentImage = m_dataManager->getCurrentImage();
        if (currentImage) {
            // 이미지 뷰어에 이미지 설정
            m_imageViewer->setImage(currentImage);
            
            // 이미지 뷰어 렌더링 (별도 창 없이 현재 영역에)
            m_imageViewer->render(false);
            
        } else {
            // 이미지가 없을 때의 안내
            ImVec2 windowSize = ImGui::GetContentRegionAvail();
            ImVec2 center = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);
            
            ImGui::SetCursorPos(center);
            ImGui::Text("No image loaded");
            ImGui::Text("Load an image to begin processing");
        }
        
        ImGui::End();
    }
    
    /**
     * @brief UI 상태 업데이트
     */
    void updateUIState() {
        if (!m_menuBar || !m_controlPanel || !m_statusBar || !m_dataManager) {
            return;
        }
        
        // 메뉴 바 상태 업데이트
        m_menuBar->setHasImage(m_dataManager->hasCurrentImage());
        m_menuBar->setCanUndo(m_dataManager->canUndo());
        m_menuBar->setCanRedo(m_dataManager->canRedo());
        m_menuBar->setIsProcessing(m_dataManager->isProcessing());
        m_menuBar->setHasUnsavedChanges(m_dataManager->hasUnsavedChanges());
        
        // 컨트롤 패널 상태 업데이트
        m_controlPanel->setProcessingState(m_dataManager->isProcessing(), 0.0f); // TODO: 진행률
        
        // 상태 바 정보 업데이트
        auto currentImage = m_dataManager->getCurrentImage();
        if (currentImage) {
            cv::Mat mat = currentImage->getOpenCVMat();
            m_statusBar->setImageInfo(mat.cols, mat.rows, mat.channels(), 
                                  mat.total() * mat.elemSize());
            m_statusBar->setImagePath(m_dataManager->getCurrentFilePath());
        }
        
        m_statusBar->setProcessingStatus(m_dataManager->getLastError());
        m_statusBar->setDataSourceType(m_dataManager->getDataSourceInfo());
        m_statusBar->setCurrentFPS(m_currentFPS);
        m_statusBar->setIsLiveSource(m_dataManager->isLiveSource());
        
        // 에러 상태 업데이트
        if (!m_dataManager->getLastError().empty()) {
            m_lastError = m_dataManager->getLastError();
        }
    }
    
    /**
     * @brief 에러 화면 렌더링
     */
    void renderErrorScreen() {
        ImGui::Begin("Error", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        
        ImGui::Text("Application failed to initialize:");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", m_lastError.c_str());
        
        ImGui::Separator();
        
        if (ImGui::Button("Retry Initialization")) {
            m_lastError.clear();
            initialize();
        }
        
        ImGui::End();
    }
    
    /**
     * @brief 리소스 정리
     */
    void cleanup() {
        m_menuBar.reset();
        m_controlPanel.reset();
        m_statusBar.reset();
        
        if (m_dataManager) {
            m_dataManager->cleanup();
            m_dataManager.reset();
        }
        
        if (m_graphicsBackend) {
            m_graphicsBackend->shutdown();
            m_graphicsBackend.reset();
        }
        
        m_initialized = false;
        std::cout << "Application cleaned up" << std::endl;
    }
};

/**
 * @brief 메인 함수
 */
int main() {
    // GLFW 초기화
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // OpenGL 버전 설정
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // 창 생성
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Data Annotation Tool", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync 활성화
    
    // ImGui 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // 도킹 활성화
    
    // 스타일 설정
    ImGui::StyleColorsDark();
    
    // 플랫폼/렌더러 백엔드 초기화
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // 애플리케이션 생성
    ImageProcessingApp app;
    
    if (!app.isInitialized()) {
        std::cerr << "Failed to initialize application: " << app.getLastError() << std::endl;
        
        // 에러 상태에서도 기본 루프는 실행 (에러 표시용)
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            app.render();
            
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }
    } else {
        // 메인 루프
        while (app.isRunning()) {
            glfwPollEvents();
            
            // ImGui 프레임 시작 (임시 - 실제로는 백엔드에서 처리)
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            // 애플리케이션 렌더링
            app.render();
            
            // ImGui 렌더링 (임시)
            ImGui::Render();
            
            // 그래픽 백엔드 프레임 종료
            // m_graphicsBackend->renderImGui();
            // m_graphicsBackend->endFrame();
            // m_graphicsBackend->swapBuffers();
            
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }
    }
    
    // 종료 처리
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}