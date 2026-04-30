/**
 * @file main_temp.cpp
 * @brief ImGui + OpenGL image viewer example with detailed comments
 * 
 * This is a simplified image viewer example to help understand the ImGui library
 * and the existing image viewing infrastructure. It demonstrates:
 * - Basic ImGui setup and initialization
 * - OpenGL context creation with GLFW
 * - Simple image loading and display
 * - Basic image controls (zoom, pan, information display)
 */

#include "imgui.h"                    // ImGui main header
#include "imgui_impl_glfw.h"           // GLFW backend for ImGui
#include "imgui_impl_opengl3.h"        // OpenGL3 backend for ImGui
#include <GLFW/glfw3.h>               // GLFW for window creation and OpenGL context
#include <opencv2/opencv.hpp>          // OpenCV for image processing
#include <iostream>
#include <string>
#include <memory>

// OpenGL headers for texture constants
#include <GL/gl.h>                     // OpenGL constants
// #include <GL/glext.h>                  // OpenGL extensions

// Include our refactored graphics backend with separated concerns
#include "src/graphics/RefactoredOpenGLBackend.cpp"

// OpenGL texture ID for displaying images
GLuint g_imageTexture = 0;
cv::Mat g_loadedImage;                // Current loaded image
float g_zoomLevel = 1.0f;             // Current zoom level
ImVec2 g_imagePosition{0, 0};         // Image position for panning
bool g_showImageInfo = true;          // Toggle for image information display
bool g_showPixelInfo = true;          // Toggle for pixel information display

/**
 * @brief Create OpenGL texture from OpenCV Mat
 * @param mat OpenCV Mat containing image data
 * @return OpenGL texture ID
 */
GLuint createTextureFromMat(const cv::Mat& mat) {
    GLuint textureID;
    
    // Generate OpenGL texture
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Set texture parameters
    // GL_LINEAR: Smooth filtering when scaling
    // GL_NEAREST: Pixelated look when scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Set texture wrapping mode
    // GL_REPEAT: Texture repeats outside 0-1 range
    // GL_CLAMP: Texture clamps at edges (more compatible than GL_CLAMP_TO_EDGE)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    // Determine image format based on number of channels
    GLenum format = GL_RGB;
    if (mat.channels() == 1) {
        format = GL_RED;
    } else if (mat.channels() == 4) {
        format = GL_RGBA;
    }
    
    // Upload image data to GPU
    // OpenCV uses BGR format, OpenGL expects RGB, so we need to convert
    cv::Mat rgbMat;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 4) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGRA2RGBA);
    } else {
        rgbMat = mat;
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, rgbMat.cols, rgbMat.rows, 0, format, GL_UNSIGNED_BYTE, rgbMat.data);
    
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return textureID;
}

/**
 * @brief Load image from file path
 * @param filepath Path to image file
 * @return Success status
 */
bool loadImageFromFile(const std::string& filepath) {
    // Load image using OpenCV
    cv::Mat image = cv::imread(filepath);
    if (image.empty()) {
        std::cerr << "Failed to load image: " << filepath << std::endl;
        return false;
    }
    
    // Clean up previous texture if exists
    if (g_imageTexture != 0) {
        glDeleteTextures(1, &g_imageTexture);
        g_imageTexture = 0;
    }
    
    // Store loaded image and create texture
    g_loadedImage = image;
    g_imageTexture = createTextureFromMat(image);
    
    // Reset zoom and position when loading new image
    g_zoomLevel = 1.0f;
    g_imagePosition = ImVec2(0, 0);
    
    std::cout << "Image loaded successfully: " << filepath << std::endl;
    std::cout << "Size: " << image.cols << "x" << image.rows << ", Channels: " << image.channels() << std::endl;
    
    return true;
}

/**
 * @brief Create a test image with gradient and shapes
 * This function creates a sample image programmatically for testing
 */
void createTestImage() {
    // Create a 400x300 RGB image
    cv::Mat testImage(300, 400, CV_8UC3);
    
    // Create gradient background
    for (int y = 0; y < testImage.rows; y++) {
        for (int x = 0; x < testImage.cols; x++) {
            // Blue to Red gradient
            testImage.at<cv::Vec3b>(y, x) = cv::Vec3b(
                (x * 255) / testImage.cols,      // Red channel
                (y * 255) / testImage.rows,      // Green channel
                255 - ((x * 255) / testImage.cols) // Blue channel
            );
        }
    }
    
    // Draw some shapes for testing
    cv::rectangle(testImage, cv::Point(50, 50), cv::Point(150, 150), cv::Scalar(255, 255, 255), 3);
    cv::circle(testImage, cv::Point(300, 100), 40, cv::Scalar(0, 255, 0), -1);
    cv::line(testImage, cv::Point(200, 200), cv::Point(350, 250), cv::Scalar(0, 0, 255), 5);
    
    // Add text
    cv::putText(testImage, "Test Image", cv::Point(100, 250), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
    
    // Clean up previous texture if exists
    if (g_imageTexture != 0) {
        glDeleteTextures(1, &g_imageTexture);
        g_imageTexture = 0;
    }
    
    // Store test image and create texture
    g_loadedImage = testImage;
    g_imageTexture = createTextureFromMat(testImage);
    
    // Reset zoom and position
    g_zoomLevel = 1.0f;
    g_imagePosition = ImVec2(0, 0);
    
    std::cout << "Test image created successfully" << std::endl;
}

/**
 * @brief Render the main image viewer window
 * This function handles all the UI for the image viewer
 */
void renderImageViewer() {
    // Begin the image viewer window
    // ImGui::Begin() creates a new window with the given title
    ImGui::Begin("Image Viewer", nullptr, ImGuiWindowFlags_MenuBar);
    
    // Add menu bar to the window
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            // Menu items for file operations
            if (ImGui::MenuItem("Load Image...")) {
                // In a real application, you would open a file dialog here
                // For now, we'll try to load a common test image
                loadImageFromFile("test.jpg"); // This will fail if file doesn't exist
            }
            if (ImGui::MenuItem("Create Test Image")) {
                createTestImage();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                // In a real application, you would set a flag to exit
                std::cout << "Exit requested" << std::endl;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            // Menu items for view options
            ImGui::Checkbox("Show Image Info", &g_showImageInfo);
            ImGui::Checkbox("Show Pixel Info", &g_showPixelInfo);
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Zoom")) {
                g_zoomLevel = 1.0f;
                g_imagePosition = ImVec2(0, 0);
            }
            if (ImGui::MenuItem("Fit to Window")) {
                // Calculate zoom to fit image in window
                if (!g_loadedImage.empty()) {
                    ImVec2 windowSize = ImGui::GetContentRegionAvail();
                    float scaleX = windowSize.x / g_loadedImage.cols;
                    float scaleY = windowSize.y / g_loadedImage.rows;
                    g_zoomLevel = std::min(scaleX, scaleY) * 0.9f; // 90% of available space
                    g_imagePosition = ImVec2(0, 0);
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    
    // Display image information if enabled
    if (g_showImageInfo && !g_loadedImage.empty()) {
        ImGui::Text("Image Info:");
        ImGui::Text("Size: %dx%d", g_loadedImage.cols, g_loadedImage.rows);
        ImGui::Text("Channels: %d", g_loadedImage.channels());
        ImGui::Text("Zoom: %.2f%%", g_zoomLevel * 100.0f);
        ImGui::Separator();
    }
    
    // Display the image if loaded
    if (g_imageTexture != 0 && !g_loadedImage.empty()) {
        // Get available space in the window
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        
        // Calculate image size with zoom
        ImVec2 imageSize(
            g_loadedImage.cols * g_zoomLevel,
            g_loadedImage.rows * g_zoomLevel
        );
        
        // Create a child window for the image with scrolling
        ImGui::BeginChild("ImageScrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        // Set the image position
        ImGui::SetCursorPos(g_imagePosition);
        
        // Display the image using ImGui::Image()
        // This function displays an OpenGL texture as an image
        ImGui::Image(
            (void*)(intptr_t)g_imageTexture,  // Cast texture ID to void*
            imageSize,                          // Size to display
            ImVec2(0, 0),                      // UV coordinates (top-left)
            ImVec2(1, 1),                      // UV coordinates (bottom-right)
            ImVec4(1, 1, 1, 1),                // Tint color (white = no tint)
            ImVec4(1, 1, 1, 0.5)               // Border color (transparent = no border)
        );
        
        // Handle mouse interactions for panning
        if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            // Pan the image when dragging with left mouse button
            ImVec2 mouseDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            g_imagePosition.x += mouseDelta.x;
            g_imagePosition.y += mouseDelta.y;
            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }
        
        // Handle mouse wheel for zooming
        if (ImGui::IsItemHovered()) {
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f) {
                // Zoom in/out with mouse wheel
                float oldZoom = g_zoomLevel;
                g_zoomLevel *= (1.0f + wheel * 0.1f);
                
                // Limit zoom range
                g_zoomLevel = std::max(0.1f, std::min(g_zoomLevel, 10.0f));
                
                // Zoom towards mouse position
                ImVec2 mousePos = ImGui::GetMousePos();
                ImVec2 windowPos = ImGui::GetWindowPos();
                ImVec2 relativeMousePos = ImVec2(mousePos.x - windowPos.x - g_imagePosition.x,
                                                mousePos.y - windowPos.y - g_imagePosition.y);
                
                // Adjust position to zoom towards mouse
                float zoomFactor = g_zoomLevel / oldZoom;
                g_imagePosition.x -= relativeMousePos.x * (zoomFactor - 1.0f);
                g_imagePosition.y -= relativeMousePos.y * (zoomFactor - 1.0f);
            }
        }
        
        // Display pixel information if enabled and mouse is over image
        if (g_showPixelInfo) {
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 relativeMousePos = ImVec2(mousePos.x - windowPos.x - g_imagePosition.x,
                                            mousePos.y - windowPos.y - g_imagePosition.y);
            
            // Convert mouse position to image coordinates
            int imageX = static_cast<int>(relativeMousePos.x / g_zoomLevel);
            int imageY = static_cast<int>(relativeMousePos.y / g_zoomLevel);
            
            // Check if mouse is within image bounds
            if (imageX >= 0 && imageX < g_loadedImage.cols && imageY >= 0 && imageY < g_loadedImage.rows) {
                ImGui::Text("Pixel: (%d, %d)", imageX, imageY);
                
                // Get pixel value
                if (g_loadedImage.channels() == 3) {
                    cv::Vec3b pixel = g_loadedImage.at<cv::Vec3b>(imageY, imageX);
                    ImGui::Text("RGB: (%d, %d, %d)", pixel[2], pixel[1], pixel[0]); // OpenCV uses BGR
                } else if (g_loadedImage.channels() == 1) {
                    uchar pixel = g_loadedImage.at<uchar>(imageY, imageX);
                    ImGui::Text("Gray: %d", pixel);
                } else if (g_loadedImage.channels() == 4) {
                    cv::Vec4b pixel = g_loadedImage.at<cv::Vec4b>(imageY, imageX);
                    ImGui::Text("RGBA: (%d, %d, %d, %d)", pixel[2], pixel[1], pixel[0], pixel[3]);
                }
            }
        }
        
        ImGui::EndChild();
        
        // Add zoom controls at the bottom
        ImGui::Separator();
        if (ImGui::Button("Zoom In")) {
            g_zoomLevel *= 1.2f;
            g_zoomLevel = std::min(g_zoomLevel, 10.0f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Zoom Out")) {
            g_zoomLevel /= 1.2f;
            g_zoomLevel = std::max(g_zoomLevel, 0.1f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            g_zoomLevel = 1.0f;
            g_imagePosition = ImVec2(0, 0);
        }
        ImGui::SameLine();
        ImGui::Text("Zoom: %.1f%%", g_zoomLevel * 100.0f);
        
    } else {
        // No image loaded - show placeholder
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        ImVec2 center = ImVec2(windowSize.x * 0.5f, windowSize.y * 0.5f);
        
        ImGui::SetCursorPos(center);
        ImGui::Text("No image loaded");
        ImGui::Text("Use File > Create Test Image to generate a sample");
        ImGui::Text("or File > Load Image... to load an image file");
    }
    
    // End the image viewer window
    ImGui::End();
}

/** 
 * @brief Main function - entry point of the application
 * 
*/

int main() {
    // Create and initialize graphics backend with one line!
    RefactoredOpenGLBackend graphics;
    if (!graphics.initialize(1200, 800, "Simple Image Viewer")) {
        std::cerr << "Failed to initialize graphics backend" << std::endl;
        return -1;
    }
    
    // Create test image on startup
    createTestImage();
    
    std::cout << "Application started successfully!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  - Left Mouse + Drag: Pan image" << std::endl;
    std::cout << "  - Mouse Wheel: Zoom in/out" << std::endl;
    std::cout << "  - Menu Bar: File and View options" << std::endl;
    
    // Main application loop
    // This runs until the window is closed
    while (!graphics.shouldClose()) {
        // Begin new frame (handles event polling, ImGui setup, screen clearing)
        graphics.beginFrame();
        
        // Render our application UI
        renderImageViewer();
        
        // End frame (handles ImGui rendering and buffer swapping)
        graphics.endFrame();
    }
    
    // Cleanup resources
    std::cout << "Shutting down application..." << std::endl;
    
    // Clean up OpenGL texture
    if (g_imageTexture != 0) {
        glDeleteTextures(1, &g_imageTexture);
    }
    
    // Graphics backend cleanup happens automatically via destructor!
    
    std::cout << "Application shutdown complete" << std::endl;
    return 0;
}