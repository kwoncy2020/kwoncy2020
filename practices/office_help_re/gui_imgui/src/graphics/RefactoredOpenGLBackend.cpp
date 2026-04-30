/**
 * @file RefactoredOpenGLBackend.cpp
 * @brief Refactored OpenGL backend with separated concerns
 * 
 * This file demonstrates how to separate concerns within a single file
 * by creating multiple focused classes instead of one monolithic class.
 * Each class has a single responsibility and works together through composition.
 * 
 * Architecture:
 * - ApplicationState: Manages application-wide state
 * - WindowManager: Handles window creation and management
 * - OpenGLRenderer: Manages rendering operations
 * - ImGuiIntegration: Handles ImGui setup and rendering
 * - SimpleOpenGLBackend: Orchestrates all components
 */

#include "core/IGraphicsBackend.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <string>

//==============================================================================
// 1. APPLICATION STATE MANAGEMENT
//==============================================================================

/**
 * @brief Manages application-wide state and settings
 * 
 * This class is responsible for managing all application state that needs
 * to persist across frames and be accessible to different components.
 * It follows the Single Responsibility Principle by only managing state.
 */
class ApplicationState {
private:
    // Initialization state
    bool m_initialized = false;
    
    // Window settings
    int m_windowWidth = 1200;
    int m_windowHeight = 800;
    std::string m_windowTitle = "Image Viewer";
    bool m_vsyncEnabled = true;
    
    // Rendering settings
    float m_clearColor[4] = {0.1f, 0.1f, 0.1f, 1.0f};  // Dark gray background
    
public:
    // Initialization state management
    bool isInitialized() const { return m_initialized; }
    void setInitialized(bool initialized) { m_initialized = initialized; }
    
    // Window state management
    int getWindowWidth() const { return m_windowWidth; }
    int getWindowHeight() const { return m_windowHeight; }
    void setWindowSize(int width, int height) { 
        m_windowWidth = width; 
        m_windowHeight = height; 
    }
    
    const std::string& getWindowTitle() const { return m_windowTitle; }
    void setWindowTitle(const std::string& title) { m_windowTitle = title; }
    
    bool isVSyncEnabled() const { return m_vsyncEnabled; }
    void setVSyncEnabled(bool enabled) { m_vsyncEnabled = enabled; }
    
    // Rendering state management
    const float* getClearColor() const { return m_clearColor; }
    void setClearColor(float r, float g, float b, float a = 1.0f) {
        m_clearColor[0] = r;
        m_clearColor[1] = g;
        m_clearColor[2] = b;
        m_clearColor[3] = a;
    }
};

//==============================================================================
// 2. WINDOW MANAGEMENT
//==============================================================================

/**
 * @brief Handles window creation, management, and event processing
 * 
 * This class is responsible for all window-related operations including
 * creation, sizing, event handling, and cleanup. It abstracts away
 * the complexities of GLFW window management.
 */
class WindowManager {
private:
    GLFWwindow* m_window = nullptr;
    bool m_initialized = false;
    
    /**
     * @brief GLFW error callback
     */
    static void errorCallback(int error, const char* description) {
        std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
    }
    
    /**
     * @brief Framebuffer size callback for window resizing
     */
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

public:
    /**
     * @brief Initialize the window system
     */
    bool initialize(int width, int height, const std::string& title) {
        // Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        
        // Set error callback
        glfwSetErrorCallback(errorCallback);
        
        // Configure GLFW
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        // Create window
        m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!m_window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        
        // Make context current
        glfwMakeContextCurrent(m_window);
        
        // Set framebuffer size callback
        glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
        
        // Enable V-Sync
        glfwSwapInterval(1);
        
        m_initialized = true;
        std::cout << "Window initialized successfully: " << width << "x" << height << std::endl;
        return true;
    }
    
    /**
     * @brief Cleanup window resources
     */
    void shutdown() {
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        if (m_initialized) {
            glfwTerminate();
            m_initialized = false;
        }
    }
    
    /**
     * @brief Check if window should close
     */
    bool shouldClose() const {
        return m_window && glfwWindowShouldClose(m_window);
    }
    
    /**
     * @brief Poll for events
     */
    void pollEvents() {
        if (m_window) {
            glfwPollEvents();
        }
    }
    
    /**
     * @brief Swap buffers
     */
    void swapBuffers() {
        if (m_window) {
            glfwSwapBuffers(m_window);
        }
    }
    
    /**
     * @brief Get window handle
     */
    void* getNativeHandle() const {
        return m_window;
    }
    
    /**
     * @brief Get window size
     */
    void getWindowSize(int& width, int& height) const {
        if (m_window) {
            glfwGetWindowSize(m_window, &width, &height);
        } else {
            width = height = 0;
        }
    }
    
    /**
     * @brief Get framebuffer size
     */
    void getFramebufferSize(int& width, int& height) const {
        if (m_window) {
            glfwGetFramebufferSize(m_window, &width, &height);
        } else {
            width = height = 0;
        }
    }
    
    /**
     * @brief Set V-Sync
     */
    void setVSync(bool enabled) {
        if (m_window) {
            glfwSwapInterval(enabled ? 1 : 0);
        }
    }
    
    /**
     * @brief Set window title
     */
    void setWindowTitle(const std::string& title) {
        if (m_window) {
            glfwSetWindowTitle(m_window, title.c_str());
        }
    }
    
    bool isInitialized() const { return m_initialized; }
};

//==============================================================================
// 3. OPENGL RENDERING
//==============================================================================

/**
 * @brief Handles OpenGL rendering operations
 * 
 * This class is responsible for all OpenGL rendering operations including
 * clearing the screen, setting up rendering state, and managing the viewport.
 * It encapsulates all OpenGL-specific rendering logic.
 */
class OpenGLRenderer {
private:
    bool m_initialized = false;
    
public:
    /**
     * @brief Initialize OpenGL rendering
     */
    bool initialize() {
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        m_initialized = true;
        std::cout << "OpenGL renderer initialized" << std::endl;
        return true;
    }
    
    /**
     * @brief Begin a new rendering frame
     */
    void beginFrame(const float* clearColor) {
        if (!m_initialized) return;
        
        // Set viewport to cover the entire window
        int width, height;
        glGetIntegerv(GL_VIEWPORT, reinterpret_cast<GLint*>(&width));
        glGetIntegerv(GL_VIEWPORT, reinterpret_cast<GLint*>(&height));
        
        // Clear screen with specified color
        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    /**
     * @brief End a rendering frame
     */
    void endFrame() {
        if (!m_initialized) return;
        
        // Flush any pending OpenGL commands
        glFlush();
    }
    
    /**
     * @brief Set viewport
     */
    void setViewport(int x, int y, int width, int height) {
        glViewport(x, y, width, height);
    }
    
    /**
     * @brief Get OpenGL version info
     */
    std::string getVersionInfo() const {
        const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        
        std::string info = "OpenGL Version: ";
        info += version ? version : "Unknown";
        info += ", Renderer: ";
        info += renderer ? renderer : "Unknown";
        
        return info;
    }
    
    bool isInitialized() const { return m_initialized; }
};

//==============================================================================
// 4. IMGUI INTEGRATION
//==============================================================================

/**
 * @brief Handles ImGui initialization and rendering
 * 
 * This class is responsible for all ImGui-related operations including
 * initialization, frame management, and rendering. It encapsulates all
 * ImGui-specific logic.
 */
class ImGuiIntegration {
private:
    bool m_initialized = false;
    
public:
    /**
     * @brief Initialize ImGui
     */
    bool initialize(GLFWwindow* window) {
        if (!window) {
            std::cerr << "Cannot initialize ImGui: no window provided" << std::endl;
            return false;
        }
        
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        
        // Enable ImGui features
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        
        m_initialized = true;
        std::cout << "ImGui initialized successfully" << std::endl;
        return true;
    }
    
    /**
     * @brief Shutdown ImGui
     */
    void shutdown() {
        if (m_initialized) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            m_initialized = false;
        }
    }
    
    /**
     * @brief Begin new ImGui frame
     */
    void newFrame() {
        if (!m_initialized) return;
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    
    /**
     * @brief End ImGui frame and render
     */
    void render() {
        if (!m_initialized) return;
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    
    bool isInitialized() const { return m_initialized; }
};

//==============================================================================
// 5. MAIN BACKEND ORCHESTRATOR
//==============================================================================

/**
 * @brief Main backend class that orchestrates all components
 * 
 * This class is the main entry point that coordinates all the separated
 * components. It provides a simple interface while delegating work to
 * the appropriate specialized classes.
 * 
 * This demonstrates the Composition pattern over inheritance.
 */
class RefactoredOpenGLBackend {
private:
    // Component instances - each with single responsibility
    std::unique_ptr<ApplicationState> m_state;
    std::unique_ptr<WindowManager> m_windowManager;
    std::unique_ptr<OpenGLRenderer> m_renderer;
    std::unique_ptr<ImGuiIntegration> m_imgui;
    
    bool m_initialized = false;

public:
    /**
     * @brief Constructor - initialize all components
     */
    RefactoredOpenGLBackend() 
        : m_state(std::make_unique<ApplicationState>())
        , m_windowManager(std::make_unique<WindowManager>())
        , m_renderer(std::make_unique<OpenGLRenderer>())
        , m_imgui(std::make_unique<ImGuiIntegration>())
        , m_initialized(false) {
    }
    
    /**
     * @brief Destructor - automatic cleanup via RAII
     */
    ~RefactoredOpenGLBackend() {
        if (m_initialized) {
            shutdown();
        }
    }
    
    /**
     * @brief Initialize the entire graphics system
     * 
     * This method orchestrates the initialization of all components
     * in the correct order and handles any failures gracefully.
     */
    bool initialize(int width, int height, const std::string& title) {
        try {
            // Update application state
            m_state->setWindowSize(width, height);
            m_state->setWindowTitle(title);
            
            // Initialize window manager
            if (!m_windowManager->initialize(width, height, title)) {
                std::cerr << "Failed to initialize window manager" << std::endl;
                return false;
            }
            
            // Initialize renderer
            if (!m_renderer->initialize()) {
                std::cerr << "Failed to initialize renderer" << std::endl;
                m_windowManager->shutdown();
                return false;
            }
            
            // Initialize ImGui
            if (!m_imgui->initialize(static_cast<GLFWwindow*>(m_windowManager->getNativeHandle()))) {
                std::cerr << "Failed to initialize ImGui" << std::endl;
                m_windowManager->shutdown();
                return false;
            }
            
            // Apply initial settings
            m_windowManager->setVSync(m_state->isVSyncEnabled());
            
            m_initialized = true;
            m_state->setInitialized(true);
            
            std::cout << "Graphics backend initialized successfully" << std::endl;
            std::cout << "Backend info: " << m_renderer->getVersionInfo() << std::endl;
            
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Exception during initialization: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief Shutdown the graphics system
     */
    void shutdown() {
        if (m_imgui) m_imgui->shutdown();
        if (m_windowManager) m_windowManager->shutdown();
        
        m_initialized = false;
        if (m_state) m_state->setInitialized(false);
        
        std::cout << "Graphics backend shutdown complete" << std::endl;
    }
    
    /**
     * @brief Check if the window should close
     */
    bool shouldClose() const {
        return m_windowManager && m_windowManager->shouldClose();
    }
    
    /**
     * @brief Begin a new rendering frame
     */
    void beginFrame() {
        if (!m_initialized) return;
        
        // Poll events
        m_windowManager->pollEvents();
        
        // Start ImGui frame
        m_imgui->newFrame();
        
        // Begin rendering
        m_renderer->beginFrame(m_state->getClearColor());
    }
    
    /**
     * @brief End the current rendering frame
     */
    void endFrame() {
        if (!m_initialized) return;
        
        // End rendering
        m_renderer->endFrame();
        
        // Render ImGui
        m_imgui->render();
        
        // Swap buffers
        m_windowManager->swapBuffers();
    }
    
    /**
     * @brief Get window handle
     */
    void* getWindowHandle() const {
        return m_windowManager ? m_windowManager->getNativeHandle() : nullptr;
    }
    
    /**
     * @brief Get window size
     */
    void getWindowSize(int& width, int& height) const {
        if (m_windowManager) {
            m_windowManager->getWindowSize(width, height);
            m_state->setWindowSize(width, height);
        } else {
            width = height = 0;
        }
    }
    
    /**
     * @brief Get framebuffer size
     */
    void getFramebufferSize(int& width, int& height) const {
        if (m_windowManager) {
            m_windowManager->getFramebufferSize(width, height);
        } else {
            width = height = 0;
        }
    }
    
    /**
     * @brief Set V-Sync
     */
    void setVSync(bool enabled) {
        if (m_windowManager) {
            m_windowManager->setVSync(enabled);
            m_state->setVSyncEnabled(enabled);
        }
    }
    
    /**
     * @brief Set window title
     */
    void setWindowTitle(const std::string& title) {
        if (m_windowManager) {
            m_windowManager->setWindowTitle(title);
            m_state->setWindowTitle(title);
        }
    }
    
    /**
     * @brief Get backend information
     */
    std::string getBackendInfo() const {
        return m_renderer ? m_renderer->getVersionInfo() : "Backend not initialized";
    }
    
    /**
     * @brief Check if initialized
     */
    bool isInitialized() const {
        return m_initialized && 
               m_state && m_state->isInitialized() &&
               m_windowManager && m_windowManager->isInitialized() &&
               m_renderer && m_renderer->isInitialized() &&
               m_imgui && m_imgui->isInitialized();
    }

private:
    // Disable copy constructor and assignment operator
    RefactoredOpenGLBackend(const RefactoredOpenGLBackend&) = delete;
    RefactoredOpenGLBackend& operator=(const RefactoredOpenGLBackend&) = delete;
};
