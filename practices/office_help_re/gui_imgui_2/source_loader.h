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


class SourceLoader{
public:
    
    // Global variables for image display
    cv::Mat m_orig_image;
    cv::Mat m_displayed_image_RGB;
    GLuint m_displayed_texture_id = 0;
    GLuint m_preview_texture_id = 0;
    int m_orig_image_width = 0;
    int m_orig_image_height = 0;

    std::vector<cv::Mat> m_preview_images;

    // Scroll state for image navigation
    struct ScrollState {
        ImVec2 scroll_pos = ImVec2(0, 0);
        ImVec2 image_size = ImVec2(0, 0);
        bool is_dragging = false;
        ImVec2 drag_start_pos = ImVec2(0, 0);
        ImVec2 drag_start_scroll = ImVec2(0, 0);
        float zoom = 1.0f;
    } m_scroll_state;

    void mat_to_texture(const cv::Mat& mat);
    cv::Mat load_image(const char* filepath);
    cv::Mat load_from_file(const char* filepath);
    
#ifdef _WIN32
    std::string open_file_dialog();
#endif

};
