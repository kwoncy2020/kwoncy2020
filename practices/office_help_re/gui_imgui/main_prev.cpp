
#include <GLFW/glfw3.h>
#include "imgui.h"
#include <opencv2/opencv.hpp>
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

// 전역 상태 관리
GLuint g_image_texture = 0;
int g_img_width = 0;
int g_img_height = 0;

// OpenCV Mat -> OpenGL Texture 변환
void UpdateTexture(const cv::Mat& frame) {
    if (frame.empty()) return;

    cv::Mat image_rgba;
    cv::cvtColor(frame, image_rgba, cv::COLOR_BGR2RGBA);

    if (g_image_texture == 0) glGenTextures(1, &g_image_texture);
    
    glBindTexture(GL_TEXTURE_2D, g_image_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_rgba.cols, image_rgba.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_rgba.data);
    
    g_img_width = image_rgba.cols;
    g_img_height = image_rgba.rows;
}




// 텍스처 ID를 저장할 변수 (0이면 로드되지 않음)
GLuint image_texture = 0;
int image_width = 0;
int image_height = 0;

// OpenCV Mat을 OpenGL Texture로 변환하는 함수
bool LoadTextureFromMat(const cv::Mat& frame, GLuint* out_texture, int* out_width, int* out_height) {
    if (frame.empty()) return false;

    // OpenCV는 BGR을 사용하므로 OpenGL용 RGB로 변환
    cv::Mat image_rgb;
    cv::cvtColor(frame, image_rgb, cv::COLOR_BGR2RGBA); // Alpha 채널 추가가 안전함

    *out_width = image_rgb.cols;
    *out_height = image_rgb.rows;

    // 기존 텍스처가 있다면 삭제 (메모리 누수 방지)
    if (*out_texture != 0) glDeleteTextures(1, out_texture);

    // OpenGL 텍스처 생성
    glGenTextures(1, out_texture);
    glBindTexture(GL_TEXTURE_2D, *out_texture);

    // 필터링 설정 (확대/축소 시 픽셀 보간)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 데이터 업로드
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *out_width, *out_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_rgb.data);

    return true;
}

void ShowImageSelector() {
    static char file_path[256] = "C:\\Users\\kwoncy\\Pictures\\5664659cbacf411c6.jpg"; // 기본 경로 예시

    ImGui::Begin("Image Viewer");

    // 1. 파일 경로 입력창
    ImGui::InputText("File Path", file_path, IM_ARRAYSIZE(file_path));

    // 2. 이미지 로드 버튼
    if (ImGui::Button("Load Image")) {
        cv::Mat img = cv::imread(file_path);
        if (!img.empty()) {
            LoadTextureFromMat(img, &image_texture, &image_width, &image_height);
        } else {
            // 실패 시 로직 (알림 등)
        }
    }

    ImGui::Separator();

    // 3. 로드된 이미지가 있으면 화면에 출력
    if (image_texture != 0) {
        ImGui::Text("Size: %d x %d", image_width, image_height);
        
        // 창 크기에 맞춰 이미지 크기 조절 (비율 유지하고 싶다면 계산 필요)
        float window_width = ImGui::GetContentRegionAvail().x;
        float aspect_ratio = (float)image_height / (float)image_width;
        
        ImVec2 display_size = ImVec2(window_width, window_width * aspect_ratio);

        // ImGui에 이미지 출력 (텍스처 ID 전달)
        ImGui::Image((void*)(intptr_t)image_texture, display_size);
        if (ImGui::IsItemHovered()) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImVec2 image_pos = ImGui::GetItemRectMin();
        // 이미지 내의 실제 픽셀 좌표 계산
        int pixel_x = (int)((mouse_pos.x - image_pos.x) / scale);
        int pixel_y = (int)((mouse_pos.y - image_pos.y) / scale);
        ImGui::SetTooltip("Pixel: %d, %d", pixel_x, pixel_y);
        }
    } else {
        ImGui::Text("No image loaded.");
    }

    ImGui::End();
}

int main() {
    // [1] GLFW 초기화
    if (!glfwInit()) return -1;

    // OpenGL 버전 설정 (3.3 Core Profile)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Data Annotation Tool", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // V-Sync 활성화

    // [2] ImGui 컨텍스트 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // [3] 플랫폼/렌더러 백엔드 초기화
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 어플리케이션 상태 변수
    char file_path[256] = "";

    // [4] 메인 루프
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // ImGui 프레임 시작
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- UI 설계 구역 ---
        {
            ImGui::Begin("Control Panel");
            
            ImGui::InputText("Image Path", file_path, IM_ARRAYSIZE(file_path));
            
            if (ImGui::Button("Load Image")) {
                cv::Mat img = cv::imread(file_path);
                if (!img.empty()) {
                    UpdateTexture(img);
                } else {
                    std::cerr << "Failed to load image: " << file_path << std::endl;
                }
            }

            ImGui::Separator();

            if (g_image_texture != 0) {
                ImGui::Text("Resolution: %d x %d", g_img_width, g_img_height);
                
                // 가용한 창 너비에 맞춰 이미지 출력
                float avail_width = ImGui::GetContentRegionAvail().x;
                float scale = avail_width / g_img_width;
                ImVec2 display_size = ImVec2(avail_width, g_img_height * scale);

                ImGui::Image((void*)(intptr_t)g_image_texture, display_size);
            }

            ImGui::End();
        }

        // [5] 렌더링
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // [6] 종료 처리
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}