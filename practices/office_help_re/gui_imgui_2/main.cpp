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

#include "source_loader.h"


class GLFW_InitManager{
public:
    GLFW_InitManager()

    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            m_isGLFWInit = false;
        else
            m_isGLFWInit = true;
    }
    ~GLFW_InitManager() {
    }
    bool isGLFWInit() const { return m_isGLFWInit; }
    GLFWwindow* getWindow() const { return m_window; }
    bool init_GLFW_Window_and_IMGUI(int width, int height, const char* title);
    void prepare_UI();
    ImGuiIO& get_IMGUI_IO() { return m_imgui_io; }
    void set_IMGUI_IO(ImGuiIO& io) { m_imgui_io = io; }
    void cleanup(SourceLoader& sourceloader);
private:
    ImGuiIO m_imgui_io;
    ImGuiContext* m_imgui_context = nullptr;
    bool m_isGLFWInit = false;
    GLFWwindow* m_window = nullptr;
    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }
};


bool GLFW_InitManager::init_GLFW_Window_and_IMGUI(int width, int height, const char* title) {
    if (m_window != nullptr) {
        //TODO : IMGUI_IMPL erase
        glfwDestroyWindow(m_window);
    }
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    m_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (m_window == nullptr) {
        return false;
    }
    
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    m_imgui_context = ImGui::CreateContext();
    m_imgui_io.IniFilename = nullptr;
    m_imgui_io = ImGui::GetIO(); (void)m_imgui_io;
    m_imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
}

void GLFW_InitManager::prepare_UI(){
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
// Simple error callback for GLFW

void GLFW_InitManager::cleanup(SourceLoader& sourceloader) {
    if (sourceloader.m_displayed_texture_id != 0) {
    if (sourceloader.m_displayed_texture_id != 0) {
        glDeleteTextures(1, &sourceloader.m_displayed_texture_id);
    }
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    
    glfwTerminate();
}




int main(int, char**)
{
    GLFW_InitManager glfw_InitManager;
    if (!glfw_InitManager.isGLFWInit())
        return 1;

    if (!glfw_InitManager.init_GLFW_Window_and_IMGUI(1280, 720, "Image Viewer"))
        return 1;

    // Try to load a default image (create a simple colored image if no file exists)
    SourceLoader sourceloader;
    if (!sourceloader.load_image("test.jpg").empty()) {
        // Create a simple test image
        sourceloader.m_displayed_image_RGB = cv::Mat(400, 400, CV_8UC3, cv::Scalar(100, 150, 200));
        cv::putText(sourceloader.m_displayed_image_RGB, "Test Image", cv::Point(100, 200), 
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        sourceloader.mat_to_texture(sourceloader.m_displayed_image_RGB);
        printf("Created default test image\n");
    }

    while (!glfwWindowShouldClose(glfw_InitManager.getWindow()))
    {
        glfw_InitManager.prepare_UI();

        // Main window with table layout
        {
            // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // 여백 0
            ImGuiWindowFlags window0_flag = 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_NoMove | 
                ImGuiWindowFlags_NoCollapse | 
                ImGuiWindowFlags_NoTitleBar |
                // ImGuiWindowFlags_NoScrollWithMouse |

                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNavFocus;

            ImGui::Begin("Image Viewer", nullptr, 
                window0_flag);
                // ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

                ImGuiTableFlags table0_flag = ImGuiTableFlags_Borders | 
                // ImGuiTableFlags_Resizable | 
                ImGuiTableFlags_NoBordersInBody |
                ImGuiTableFlags_ScrollY |
                ImGuiTableFlags_ScrollX ;
                // ImGuiTableFlags_NoSavedSettings;
                // ImGuiTableFlags_SizingStretchProp |
                // ImGuiTableFlags_SizingStretchSame |
                // ImGuiTableFlags_SizingFixedFit |
            ImVec2 availableTable0Region = ImGui::GetContentRegionAvail();
            if (ImGui::BeginTable("Main###Layout", 2, 
                table0_flag,
                // ImVec2(0.0f, availableTable0Region.y))) {
                // ImVec2(0.0f, availableTable0Region.y))) {
                // ImVec2(0.0f, 0.0f))) {
                ImVec2(availableTable0Region))) {
                
                // Image panel (2/3 width)
                ImGui::TableSetupColumn("Image", ImGuiTableColumnFlags_WidthStretch, 2.0f);
                // Info panel (1/3 width)
                // ImVec2 availableTableColumn0Region = ImGui::GetContentRegionAvail();
                ImGui::TableSetupColumn("Info", ImGuiTableColumnFlags_WidthStretch, 1.0f);
                
                ImVec2 availableTableNextRow0Region = ImGui::GetContentRegionAvail();
                ImGui::TableNextRow(ImGuiTableRowFlags_None, availableTableNextRow0Region.y);
                // ImGui::TableNextRow();
                // ImGui::TableNextRow(ImGuiTableRowFlags_None, -FLT_MAX);
                
                // Image panel
                ImGui::TableSetColumnIndex(0);
                if (sourceloader.m_displayed_texture_id != 0) {
                    ImVec2 availableTableColumn0Region1 = ImGui::GetContentRegionAvail();
       
                    // Simple scrolling with visible scrollbars
                    ImGuiWindowFlags windowChild_flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar;
                    if (ImGui::BeginChild("ImageScrollArea", availableTableColumn0Region1, true, windowChild_flags)) {
                        ImGui::Image((void*)(intptr_t)sourceloader.m_displayed_texture_id, 
                                //    sourceloader.m_scroll_state.image_size);
                                   ImVec2(sourceloader.m_orig_image_width, sourceloader.m_orig_image_height));
                        
                        // Mouse drag scrolling
                        // if (ImGui::IsItemHovered() && ImGui::IsMouseDragging(0)) {
                        if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0)) {
                            ImVec2 mouse_delta = ImGui::GetMouseDragDelta(0);
                            ImGui::SetScrollX(ImGui::GetScrollX() - mouse_delta.x);
                            ImGui::SetScrollY(ImGui::GetScrollY() - mouse_delta.y);
                            ImGui::ResetMouseDragDelta(0);
                        }
   
                    }
                    ImGui::EndChild();
                    
                    
                } else {
                    ImGui::Text("No image loaded");
                }
                
                // Info panel
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Image Information");
                ImGui::Separator();
                
                // File operations
                if (ImGui::Button("Load Image")) {
                    sourceloader.load_from_file(""); // Empty string triggers file dialog
                }
                
                ImGui::Spacing();
                
                // // Image dimensions
                // ImGui::Text("Dimensions:");
                // ImGui::Text("  Original: %dx%d", sourceloader.m_orig_image_width, sourceloader.m_orig_image_height);
                // if (sourceloader.m_displayed_texture_id != 0) {
                //     ImGui::Text("  Display: %.0fx%.0f", sourceloader.m_orig_image_width, sourceloader.m_orig_image_height);
                //     // ImGui::Text("  Zoom: %.1fx", sourceloader.m_scroll_state.zoom);
                // }
                
                ImGui::Spacing();
                
                // // Scroll position
                // if (sourceloader.m_displayed_texture_id != 0) {
                //     ImGui::Text("Scroll Position:");
                //     ImGui::Text("  X: %.0f", sourceloader.m_scroll_state.scroll_pos.x);
                //     ImGui::Text("  Y: %.0f", sourceloader.m_scroll_state.scroll_pos.y);
                // }
                
                ImGui::Spacing();
                
                // Controls
                ImGui::Text("Controls:");
                ImGui::Text("  • Drag to pan");
                ImGui::Text("  • Scroll to zoom");
                
                // if (ImGui::Button("Reset View")) {
                //     sourceloader.m_scroll_state.scroll_pos = ImVec2(0, 0);
                //     sourceloader.m_scroll_state.zoom = 1.0f;
                // }
                
                ImGui::Spacing();
                
                // Performance
                ImGui::Text("Performance:");
                ImGui::Text("  FPS: %.1f", glfw_InitManager.get_IMGUI_IO().Framerate);
                
                ImGui::EndTable();
            }
            
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;   
        glfwGetFramebufferSize(glfw_InitManager.getWindow(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(glfw_InitManager.getWindow());
    }



    glfw_InitManager.cleanup(sourceloader);

    return 0;
}
