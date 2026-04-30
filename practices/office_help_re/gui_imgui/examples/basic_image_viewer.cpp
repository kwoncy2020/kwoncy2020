/**
 * @file basic_image_viewer.cpp
 * @brief 백엔드 중립적 이미지 뷰어 사용 예제
 * 
 * 이 예제는 새로운 이미지 뷰어 아키텍처를 어떻게 사용하는지 보여줍니다.
 * OpenGL 백엔드를 사용하여 이미지를 로드하고 표시합니다.
 */

#include "image/ImageViewer.h"
#include "backends/OpenGLImageLoader.h"
#include "sources/ImageFileSource.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

class BasicImageApp {
private:
    std::unique_ptr<OpenGLImageLoader> m_imageLoader;
    std::unique_ptr<ImageViewer> m_imageViewer;
    std::shared_ptr<Image> m_currentImage;
    
    // UI 상태
    char m_filePath[256] = "";
    bool m_showDemo = false;

public:
    BasicImageApp() {
        // 이미지 로더 초기화
        m_imageLoader = std::make_unique<OpenGLImageLoader>();
        if (!m_imageLoader->initialize()) {
            std::cerr << "Failed to initialize OpenGL image loader" << std::endl;
            return;
        }
        
        // 이미지 뷰어 생성
        m_imageViewer = std::make_unique<ImageViewer("Image Viewer");
        
        // 마우스 이벤트 콜백 설정
        m_imageViewer->setMouseClickCallback([this](int x, int y, int button) {
            std::cout << "Mouse click at pixel: (" << x << ", " << y << ")" << std::endl;
        });
        
        m_imageViewer->setMouseMoveCallback([this](int x, int y) {
            // 마우스 이동 처리 (필요 시)
        });
        
        m_imageViewer->setMouseScrollCallback([this](float delta) {
            std::cout << "Mouse scroll: " << delta << std::endl;
        });
    }
    
    ~BasicImageApp() {
        cleanup();
    }
    
    void render() {
        // 메인 컨트롤 패널
        ImGui::Begin("Image Control Panel");
        
        // 파일 경로 입력
        ImGui::InputText("Image Path", m_filePath, sizeof(m_filePath));
        
        // 이미지 로드 버튼
        if (ImGui::Button("Load Image")) {
            loadImage(m_filePath);
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Load Sample")) {
            // 샘플 이미지 로드
            loadSampleImage();
        }
        
        ImGui::Separator();
        
        // 현재 이미지 정보
        if (m_currentImage && m_currentImage->isLoaded()) {
            const ImageData& data = m_currentImage->getData();
            ImGui::Text("Current Image:");
            ImGui::Text("  Size: %d x %d", data.width, data.height);
            ImGui::Text("  Channels: %d", data.channels);
            ImGui::Text("  Name: %s", m_currentImage->getName().c_str());
            
            // 뷰어 컨트롤
            if (ImGui::Button("Fit to Window")) {
                m_imageViewer->setViewMode(ImageViewer::ViewMode::FitToWindow);
            }
            ImGui::SameLine();
            if (ImGui::Button("Actual Size")) {
                m_imageViewer->setViewMode(ImageViewer::ViewMode::ActualSize);
            }
            
            // 확대/축소 슬라이더
            float scale = m_imageViewer->getScale();
            if (ImGui::SliderFloat("Zoom", &scale, 0.1f, 5.0f, "%.2f")) {
                m_imageViewer->setScale(scale);
            }
            
            // 프리뷰 토글
            bool usePreview = false; // 현재 프리뷰 기능은 구현 중
            ImGui::Checkbox("Use Preview", &usePreview);
            
        } else {
            ImGui::Text("No image loaded");
            ImGui::Text("Enter a file path or load a sample image");
        }
        
        ImGui::Separator();
        
        // 옵션
        ImGui::Checkbox("Show ImGui Demo", &m_showDemo);
        
        ImGui::End();
        
        // 이미지 뷰어 렌더링
        if (m_currentImage && m_currentImage->isLoaded()) {
            m_imageViewer->render(true);
        }
        
        // ImGui 데모 창
        if (m_showDemo) {
            ImGui::ShowDemoWindow(&m_showDemo);
        }
    }
    
    void loadImage(const std::string& filepath) {
        if (filepath.empty()) {
            std::cerr << "Empty file path" << std::endl;
            return;
        }
        
        // 데이터 소스 생성
        auto dataSource = std::make_unique<ImageFileSource>();
        if (!dataSource->open(filepath)) {
            std::cerr << "Failed to open image file: " << filepath << std::endl;
            std::cerr << "Error: " << dataSource->getLastError() << std::endl;
            return;
        }
        
        // 이미지 로드
        void* textureId = m_imageLoader->loadImageFromFile(filepath);
        if (!textureId) {
            std::cerr << "Failed to load image texture" << std::endl;
            return;
        }
        
        // 이미지 객체 생성
        m_currentImage = std::make_shared<Image>(filepath);
        m_currentImage->setTextureId(textureId);
        
        // CPU 데이터도 로드 (처리용)
        cv::Mat frame;
        if (dataSource->readNextFrame(frame)) {
            m_currentImage->setCpuData(frame);
            
            // 크기 정보 설정
            m_currentImage->setSize(frame.cols, frame.rows, frame.channels());
        }
        
        // 뷰어에 이미지 설정
        m_imageViewer->setImage(m_currentImage);
        
        std::cout << "Successfully loaded image: " << filepath << std::endl;
    }
    
    void loadSampleImage() {
        // 샘플 이미지 생성 (테스트용)
        cv::Mat sampleImage = cv::Mat::zeros(400, 600, CV_8UC3);
        
        // 그라데이션 패턴 생성
        for (int y = 0; y < sampleImage.rows; ++y) {
            for (int x = 0; x < sampleImage.cols; ++x) {
                sampleImage.at<cv::Vec3b>(y, x) = cv::Vec3b(
                    (x * 255) / sampleImage.cols,      // Red
                    (y * 255) / sampleImage.rows,      // Green
                    128                                // Blue
                );
            }
        }
        
        // 텍스트 추가
        cv::putText(sampleImage, "Sample Image", cv::Point(50, 50), 
                   cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        
        // 텍스처 생성
        int width, height;
        void* textureId = m_imageLoader->createTextureFromMat(sampleImage, &width, &height);
        if (!textureId) {
            std::cerr << "Failed to create sample texture" << std::endl;
            return;
        }
        
        // 이미지 객체 생성
        m_currentImage = std::make_shared<Image>(sampleImage, "Sample Image");
        m_currentImage->setTextureId(textureId);
        m_currentImage->setSize(width, height, sampleImage.channels());
        
        // 뷰어에 이미지 설정
        m_imageViewer->setImage(m_currentImage);
        
        std::cout << "Successfully created sample image" << std::endl;
    }
    
    void cleanup() {
        m_imageViewer.reset();
        m_currentImage.reset();
        m_imageLoader.reset();
    }
};

// 메인 함수
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
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Backend-Neutral Image Viewer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync
    
    // ImGui 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // 스타일 설정
    ImGui::StyleColorsDark();
    
    // 플랫폼/렌더러 백엔드 초기화
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // 애플리케이션 생성
    BasicImageApp app;
    
    // 메인 루프
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // ImGui 프레임 시작
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // 애플리케이션 렌더링
        app.render();
        
        // 렌더링
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    
    // 종료 처리
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
