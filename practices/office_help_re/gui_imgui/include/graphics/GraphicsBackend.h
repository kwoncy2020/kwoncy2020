#pragma once

/**
 * @file GraphicsBackend.h
 * @brief Abstract interface for graphics rendering backends
 * 
 * This file defines the abstract interface that all graphics backends must implement.
 * It provides abstraction over different rendering APIs (OpenGL, DirectX, Vulkan, etc.)
 * allowing the application to switch between backends without changing the main code.
 */

#include <string>
#include <memory>

/**
 * @brief Abstract base class for graphics rendering backends
 * 
 * This class defines the common interface that all graphics backends must implement.
 * It handles window creation, context management, and rendering operations.
 * 
 * Design Principles:
 * - RAII: Resources are automatically managed
 * - Interface Segregation: Only essential methods are included
 * - Dependency Inversion: Application depends on abstraction, not concrete implementation
 * 
 * Usage Example:
 * @code
 * std::unique_ptr<GraphicsBackend> backend = std::make_unique<OpenGLBackend>();
 * if (!backend->initialize(1200, 800, "Image Viewer")) {
 *     return -1;
 * }
 * 
 * while (backend->shouldClose()) {
 *     backend->beginFrame();
 *     // Render UI and graphics
 *     backend->endFrame();
 * }
 * 
 * // Automatic cleanup via destructor
 * @endcode
 */
class GraphicsBackend {
public:
    /**
     * @brief Virtual destructor for proper cleanup
     * 
     * Ensures that derived classes are properly destroyed and resources are cleaned up.
     */
    virtual ~GraphicsBackend() = default;

    /**
     * @brief Initialize the graphics backend
     * 
     * Creates window, sets up graphics context, and initializes all necessary resources.
     * This must be called before any other graphics operations.
     * 
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @param title Window title
     * @return true if initialization succeeded, false otherwise
     * 
     * @note This method should handle all platform-specific initialization
     * @throws std::runtime_error if critical initialization fails
     */
    virtual bool initialize(int width, int height, const std::string& title) = 0;

    /**
     * @brief Check if the window should close
     * 
     * @return true if the user requested to close the window, false otherwise
     */
    virtual bool shouldClose() const = 0;

    /**
     * @brief Begin a new rendering frame
     * 
     * Prepares the graphics system for a new frame of rendering.
     * This should be called at the beginning of each frame.
     * 
     * Typical operations:
     * - Poll input events
     * - Clear screen
     * - Start ImGui frame
     */
    virtual void beginFrame() = 0;

    /**
     * @brief End the current rendering frame
     * 
     * Finalizes the current frame and presents it to the screen.
     * This should be called at the end of each frame.
     * 
     * Typical operations:
     * - Render ImGui
     * - Swap buffers
     * - Sync with display refresh rate
     */
    virtual void endFrame() = 0;

    /**
     * @brief Get the window handle
     * 
     * Returns the platform-specific window handle.
     * This is useful for platform-specific operations or integration with other libraries.
     * 
     * @return void* Platform-specific window handle (e.g., GLFWwindow*, HWND, etc.)
     */
    virtual void* getWindowHandle() const = 0;

    /**
     * @brief Get the framebuffer size
     * 
     * Returns the actual size of the framebuffer, which may differ from window size
     * on high-DPI displays or with certain window configurations.
     * 
     * @param width Output parameter for framebuffer width
     * @param height Output parameter for framebuffer height
     */
    virtual void getFramebufferSize(int& width, int& height) const = 0;

    /**
     * @brief Set the swap interval (V-Sync)
     * 
     * Controls whether the rendering is synchronized with the display refresh rate.
     * 
     * @param interval 0 for unlimited FPS, 1 for V-Sync enabled
     */
    virtual void setSwapInterval(int interval) = 0;

    /**
     * @brief Get the current swap interval
     * 
     * @return Current swap interval value
     */
    virtual int getSwapInterval() const = 0;

    /**
     * @brief Enable or disable vertical synchronization
     * 
     * Convenience method for V-Sync control.
     * 
     * @param enabled true to enable V-Sync, false to disable
     */
    virtual void setVSync(bool enabled) {
        setSwapInterval(enabled ? 1 : 0);
    }

    /**
     * @brief Check if V-Sync is currently enabled
     * 
     * @return true if V-Sync is enabled, false otherwise
     */
    virtual bool isVSyncEnabled() const {
        return getSwapInterval() > 0;
    }

    /**
     * @brief Get the backend type name
     * 
     * Returns a string identifying the type of graphics backend.
     * Useful for debugging and logging.
     * 
     * @return String containing the backend name (e.g., "OpenGL", "DirectX", "Vulkan")
     */
    virtual std::string getBackendName() const = 0;

    /**
     * @brief Get the backend version information
     * 
     * Returns version information for the graphics backend and underlying API.
     * 
     * @return String containing version information
     */
    virtual std::string getBackendVersion() const = 0;

protected:
    /**
     * @brief Protected constructor to prevent direct instantiation
     * 
     * This class is meant to be used as a base class, so the constructor is protected.
     */
    GraphicsBackend() = default;

private:
    // Disable copy constructor and assignment operator
    GraphicsBackend(const GraphicsBackend&) = delete;
    GraphicsBackend& operator=(const GraphicsBackend&) = delete;
};

/**
 * @brief Factory function for creating graphics backends
 * 
 * This function provides a convenient way to create graphics backend instances
 * without needing to know the specific concrete class names.
 * 
 * @param backendType Type of backend to create ("OpenGL", "DirectX", "Vulkan", etc.)
 * @return std::unique_ptr<GraphicsBackend> Pointer to the created backend
 * 
 * @throws std::invalid_argument if backendType is not supported
 */
std::unique_ptr<GraphicsBackend> createGraphicsBackend(const std::string& backendType);
