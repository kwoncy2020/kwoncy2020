/**
 * @file SimpleOpenGLBackend.cpp
 * @brief Simple OpenGL backend wrapper for easy initialization
 * 
 * This file provides a simplified wrapper around the existing OpenGL backend
 * to make initialization and cleanup as simple as possible.
 * It demonstrates RAII pattern and clean resource management.
 */

#include "core/IGraphicsBackend.h"
#include "graphics/OpenGLBackend.h"
#include <iostream>
#include <memory>

#include "graphics/OpenGLBackend.h"

/**
 * @brief Simple OpenGL backend wrapper class
 * 
 * This class provides a very simple interface for OpenGL initialization
 * and cleanup using RAII pattern. It wraps the existing OpenGLBackend
 * to provide a cleaner, more intuitive API.
 * 
 * Key Features:
 * - RAII: Automatic resource management
 * - Simple one-line initialization
 * - Exception safety
 * - Clean error handling
 * 
 * Usage Example:
 * @code
 * SimpleOpenGLBackend graphics;
 * if (!graphics.initialize(1200, 800, "Image Viewer")) {
 *     return -1;
 * }
 * 
 * while (!graphics.shouldClose()) {
 *     graphics.beginFrame();
 *     // Your rendering code here
 *     graphics.endFrame();
 * }
 * 
 * // Automatic cleanup via destructor
 * @endcode
 */
class SimpleOpenGLBackend {
private:
    std::unique_ptr<OpenGLBackend> m_backend;  // OpenGL backend implementation
    bool m_initialized;                         // Initialization status

public:
    /**
     * @brief Constructor
     * 
     * Initializes the backend pointer and sets initialization status to false.
     */
    SimpleOpenGLBackend() 
        : m_backend(nullptr), m_initialized(false) {
    }

    /**
     * @brief Destructor
     * 
     * Automatically cleans up resources using RAII pattern.
     * If the backend was initialized, it will be properly shutdown.
     */
    ~SimpleOpenGLBackend() {
        if (m_initialized && m_backend) {
            m_backend->shutdown();
            m_backend->shutdownImGui();
        }
    }

    /**
     * @brief Initialize the graphics backend
     * 
     * Creates and initializes the OpenGL backend with the specified parameters.
     * This method handles all the complex initialization steps internally.
     * 
     * @param width Window width in pixels
     * @param height Window height in pixels  
     * @param title Window title string
     * @return true if initialization succeeded, false otherwise
     * 
     * This method performs the following steps:
     * 1. Creates OpenGL backend instance
     * 2. Initializes the backend (window creation, OpenGL context)
     * 3. Initializes ImGui integration
     * 4. Sets up default OpenGL state
     * 
     * @note If initialization fails, all resources are automatically cleaned up
     */
    bool initialize(int width, int height, const std::string& title) {
        try {
            // Create OpenGL backend instance
            m_backend = std::make_unique<OpenGLBackend>();
            
            // Initialize the graphics backend
            if (!m_backend->initialize(width, height, title)) {
                std::cerr << "Failed to initialize OpenGL backend" << std::endl;
                return false;
            }
            
            // Initialize ImGui integration
            if (!m_backend->initializeImGui()) {
                std::cerr << "Failed to initialize ImGui" << std::endl;
                m_backend->shutdown();
                return false;
            }
            
            // Set up default rendering state
            m_backend->setRenderTarget();
            m_backend->setVSync(true);  // Enable V-Sync for smooth rendering
            
            m_initialized = true;
            std::cout << "Graphics backend initialized successfully" << std::endl;
            std::cout << "Backend info: " << m_backend->getBackendInfo() << std::endl;
            
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Exception during initialization: " << e.what() << std::endl;
            return false;
        }
    }

    /**
     * @brief Check if the window should close
     * 
     * @return true if the user requested to close the window, false otherwise
     */
    bool shouldClose() const {
        return m_backend && m_backend->shouldClose();
    }

    /**
     * @brief Begin a new rendering frame
     * 
     * Prepares the graphics system for a new frame.
     * This handles event polling and starting the ImGui frame.
     */
    void beginFrame() {
        if (!m_backend) return;
        
        // Poll input events
        m_backend->pollEvents();
        
        // Start new ImGui frame
        m_backend->newFrame();
        
        // Set up rendering target and clear screen
        m_backend->setRenderTarget();
        m_backend->clear(0.1f, 0.1f, 0.1f, 1.0f);  // Dark gray background
    }

    /**
     * @brief End the current rendering frame
     * 
     * Finalizes the current frame and presents it to the screen.
     * This handles ImGui rendering and buffer swapping.
     */
    void endFrame() {
        if (!m_backend) return;
        
        // End ImGui frame and render
        m_backend->endFrame();
        
        // Swap buffers to present the frame
        m_backend->swapBuffers();
    }

    /**
     * @brief Get the native window handle
     * 
     * @return void* Platform-specific window handle (GLFWwindow*)
     */
    void* getWindowHandle() const {
        return m_backend ? m_backend->getNativeWindowHandle() : nullptr;
    }

    /**
     * @brief Get the window size
     * 
     * @param width Output parameter for window width
     * @param height Output parameter for window height
     */
    void getWindowSize(int& width, int& height) const {
        if (m_backend) {
            m_backend->getWindowSize(&width, &height);
        } else {
            width = height = 0;
        }
    }

    /**
     * @brief Get the framebuffer size
     * 
     * @param width Output parameter for framebuffer width
     * @param height Output parameter for framebuffer height
     */
    void getFramebufferSize(int& width, int& height) const {
        if (m_backend) {
            m_backend->getFramebufferSize(&width, &height);
        } else {
            width = height = 0;
        }
    }

    /**
     * @brief Set V-Sync enabled/disabled
     * 
     * @param enabled true to enable V-Sync, false to disable
     */
    void setVSync(bool enabled) {
        if (m_backend) {
            m_backend->setVSync(enabled);
        }
    }

    /**
     * @brief Set window title
     * 
     * @param title New window title
     */
    void setWindowTitle(const std::string& title) {
        if (m_backend) {
            m_backend->setWindowTitle(title);
        }
    }

    /**
     * @brief Get backend information
     * 
     * @return std::string Backend version and capability information
     */
    std::string getBackendInfo() const {
        return m_backend ? m_backend->getBackendInfo() : "Backend not initialized";
    }

    /**
     * @brief Check if the backend is initialized
     * 
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const {
        return m_initialized && m_backend && m_backend->isInitialized();
    }

private:
    // Disable copy constructor and assignment operator
    SimpleOpenGLBackend(const SimpleOpenGLBackend&) = delete;
    SimpleOpenGLBackend& operator=(const SimpleOpenGLBackend&) = delete;
};
