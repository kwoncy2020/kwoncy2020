#include "graphics/OpenGLBackend.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <sstream>

OpenGLBackend::OpenGLBackend() 
    : m_window(nullptr), m_windowWidth(0), m_windowHeight(0), 
      m_initialized(false), m_vsyncEnabled(true) {
}

OpenGLBackend::~OpenGLBackend() {
    shutdown();
}

bool OpenGLBackend::initialize(int width, int height, const std::string& title) {
    if (m_initialized) {
        return true;
    }
    
    m_windowWidth = width;
    m_windowHeight = height;
    m_windowTitle = title;
    
    // GLFW 에러 콜백 설정
    glfwSetErrorCallback(errorCallback);
    
    // GLFW 초기화
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // OpenGL 버전 설정
    setupOpenGLVersion();
    
    // 창 생성
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    // 컨텍스트 설정
    glfwMakeContextCurrent(m_window);
    
    // V-Sync 설정
    glfwSwapInterval(m_vsyncEnabled ? 1 : 0);
    
    // 창 크기 변경 콜백 설정
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    
    // OpenGL 상태 설정
    setupOpenGLState();
    
    // GLEW 초기화 (필요한 경우)
    // glewInit() 또는 gladLoadGL() 등
    
    m_initialized = true;
    std::cout << "OpenGL backend initialized successfully" << std::endl;
    return true;
}

void OpenGLBackend::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    // ImGui 종료
    shutdownImGui();
    
    // 창 삭제
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    
    // GLFW 종료
    glfwTerminate();
    
    m_initialized = false;
}

void OpenGLBackend::newFrame() {
    if (!m_initialized) {
        return;
    }
    
    // ImGui 새 프레임 시작
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void OpenGLBackend::endFrame() {
    if (!m_initialized) {
        return;
    }
    
    // ImGui 렌더링 완료
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OpenGLBackend::setRenderTarget() {
    if (!m_initialized) {
        return;
    }
    
    // 뷰포트 설정
    int width, height;
    getFramebufferSize(&width, &height);
    glViewport(0, 0, width, height);
}

void OpenGLBackend::clear(float r, float g, float b, float a) {
    if (!m_initialized) {
        return;
    }
    
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool OpenGLBackend::initializeImGui() {
    if (!m_initialized || !m_window) {
        return false;
    }
    
    // ImGui 컨텍스트 생성
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    // ImGui IO 설정
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // 스타일 설정
    ImGui::StyleColorsDark();
    
    // 플랫폼/렌더러 백엔드 초기화
    if (!ImGui_ImplGlfw_InitForOpenGL(m_window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend" << std::endl;
        return false;
    }
    
    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        std::cerr << "Failed to initialize ImGui OpenGL backend" << std::endl;
        return false;
    }
    
    std::cout << "ImGui initialized successfully" << std::endl;
    return true;
}

void OpenGLBackend::shutdownImGui() {
    if (!m_initialized) {
        return;
    }
    
    // ImGui 종료
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool OpenGLBackend::shouldClose() const {
    if (!m_initialized || !m_window) {
        return true;
    }
    
    return glfwWindowShouldClose(m_window);
}

void OpenGLBackend::pollEvents() {
    if (!m_initialized) {
        return;
    }
    
    glfwPollEvents();
}

void OpenGLBackend::swapBuffers() {
    if (!m_initialized || !m_window) {
        return;
    }
    
    glfwSwapBuffers(m_window);
}

GraphicsBackendType OpenGLBackend::getType() const {
    return GraphicsBackendType::OpenGL;
}

std::string OpenGLBackend::getBackendInfo() const {
    std::ostringstream oss;
    oss << "OpenGL Backend\n";
    
    if (m_initialized) {
        const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        
        oss << "Version: " << (version ? version : "Unknown") << "\n";
        oss << "Renderer: " << (renderer ? renderer : "Unknown") << "\n";
        oss << "Vendor: " << (vendor ? vendor : "Unknown") << "\n";
        oss << "Window: " << m_windowWidth << "x" << m_windowHeight;
    } else {
        oss << "Not initialized";
    }
    
    return oss.str();
}

bool OpenGLBackend::isInitialized() const {
    return m_initialized;
}

void OpenGLBackend::getWindowSize(int* width, int* height) const {
    if (!m_initialized || !m_window) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }
    
    glfwGetWindowSize(m_window, width, height);
}

void OpenGLBackend::getFramebufferSize(int* width, int* height) const {
    if (!m_initialized || !m_window) {
        if (width) *width = 0;
        if (height) *height = 0;
        return;
    }
    
    glfwGetFramebufferSize(m_window, width, height);
}

void OpenGLBackend::setVSync(bool enabled) {
    if (!m_initialized || !m_window) {
        return;
    }
    
    m_vsyncEnabled = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

void OpenGLBackend::setWindowTitle(const std::string& title) {
    if (!m_initialized || !m_window) {
        return;
    }
    
    m_windowTitle = title;
    glfwSetWindowTitle(m_window, title.c_str());
}

void* OpenGLBackend::getNativeWindowHandle() const {
    return static_cast<void*>(m_window);
}

void* OpenGLBackend::getNativeContext() const {
    // OpenGL은 컨텍스트가 현재 스레드에 바인딩되어 있음
    // 필요한 경우 glfwGetCurrentContext() 사용 가능
    return nullptr;
}

// Private 메서드

void OpenGLBackend::setupOpenGLVersion() {
    // OpenGL 3.3 Core Profile 설정
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // macOS 호환성
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    
    // 추가 창 힌트
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GL_TRUE);
}

void OpenGLBackend::setupOpenGLState() {
    // 기본 OpenGL 상태 설정
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 깊이 테스트 설정 (필요한 경우)
    // glEnable(GL_DEPTH_TEST);
    // glDepthFunc(GL_LEQUAL);
    
    // 컬링 설정 (필요한 경우)
    // glDisable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    
    // 뷰포트 초기 설정
    int width, height;
    getFramebufferSize(&width, &height);
    glViewport(0, 0, width, height);
    
    checkGLError("setupOpenGLState");
}

void OpenGLBackend::errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
}

void OpenGLBackend::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // 뷰포트 업데이트
    glViewport(0, 0, width, height);
    
    // OpenGLBackend 인스턴스 포인터 얻기 (필요한 경우)
    // OpenGLBackend* backend = static_cast<OpenGLBackend*>(glfwGetWindowUserPointer(window));
    // backend->m_windowWidth = width;
    // backend->m_windowHeight = height;
}

bool OpenGLBackend::checkGLError(const std::string& operation) const {
    GLenum error;
    bool hasError = false;
    
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL Error in " << operation << ": " << error << std::endl;
        hasError = true;
    }
    
    return !hasError;
}
