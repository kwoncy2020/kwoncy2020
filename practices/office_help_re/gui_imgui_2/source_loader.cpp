#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>

// OpenCV headers for image processing
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// Windows API for file dialog
#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#pragma comment(lib, "comdlg32.lib")
#endif

#include "source_loader.h"

#ifdef _WIN32
// Open file dialog and return selected file path
std::string SourceLoader::open_file_dialog() {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp;*.tiff;*.tif\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    
    // Display the Open dialog box
    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(szFile);
    }
    
    return ""; // User cancelled or error
}
#endif

// Convert OpenCV Mat to OpenGL texture
void SourceLoader::mat_to_texture(const cv::Mat& mat) {
    if (m_displayed_texture_id != 0) {
    if (m_displayed_texture_id != 0) {
        glDeleteTextures(1, &m_displayed_texture_id);
    }
    }
    
    glGenTextures(1, &m_displayed_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_displayed_texture_id);
    glBindTexture(GL_TEXTURE_2D, m_displayed_texture_id);
    
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mat.cols, mat.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, mat.data);
    
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    
    m_orig_image_width = m_orig_image.cols;
    m_orig_image_height = m_orig_image.rows;
}


cv::Mat SourceLoader::load_from_file(const char* filepath) {
    std::string file_path;
    
    // If no filepath provided, open file dialog
    if (filepath == nullptr || strlen(filepath) == 0) {
#ifdef _WIN32
        file_path = open_file_dialog();
        if (file_path.empty()) {
            printf("User cancelled file selection\n");
            return cv::Mat();
        }
#else
        printf("File dialog not implemented for this platform\n");
        return cv::Mat();
#endif
    } else {
        file_path = filepath;
    }
    
    cv::Mat img = cv::imread(file_path);
    if (img.empty()) {
        printf("Failed to load image: %s\n", file_path.c_str());
        return cv::Mat();
    }
    
    // Update member variables
    m_orig_image = img;
    cv::cvtColor(img, m_displayed_image_RGB, cv::COLOR_BGR2RGB);
    mat_to_texture(m_displayed_image_RGB);
    
    m_orig_image_width = img.cols;
    m_orig_image_height = img.rows;
    
    // Reset scroll state
    m_scroll_state.scroll_pos = ImVec2(0, 0);
    m_scroll_state.zoom = 1.0f;
    
    printf("Loaded image: %s (%dx%d)\n", file_path.c_str(), m_orig_image_width, m_orig_image_height);
    return m_displayed_image_RGB;
}

cv::Mat SourceLoader::load_image(const char* filepath) {
    cv::Mat img = cv::imread(filepath);
    if (img.empty()) {
        printf("Failed to load image: %s\n", filepath);
        return cv::Mat();
    }
    
    m_orig_image = img;
    // Convert BGR to RGB for OpenGL
    cv::cvtColor(img, m_displayed_image_RGB, cv::COLOR_BGR2RGB);
    mat_to_texture(m_displayed_image_RGB);
    
    printf("Loaded image: %s (%dx%d)\n", filepath, m_orig_image_width, m_orig_image_height);
    return m_displayed_image_RGB;
}

