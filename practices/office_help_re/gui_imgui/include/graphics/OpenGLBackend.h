#pragma once

#include "core/IGraphicsBackend.h"
#include <GLFW/glfw3.h>

/**
 * @brief OpenGL 그래픽 백엔드 구현
 * 
 * 이 클래스는 IGraphicsBackend 인터페이스의 OpenGL 구현을 제공합니다.
 * GLFW를 사용하여 창을 생성하고 OpenGL 컨텍스트를 관리합니다.
 * 
 * 주요 기능:
 * - OpenGL 컨텍스트 초기화 및 관리
 * - GLFW 창 생성 및 이벤트 처리
 * - ImGui OpenGL 백엔드 연동
 * - V-Sync 및 기본 OpenGL 설정
 */
class OpenGLBackend : public IGraphicsBackend {
private:
    GLFWwindow* m_window;              // GLFW 윈도우 핸들
    int m_windowWidth;                 // 창 너비
    int m_windowHeight;                // 창 높이
    std::string m_windowTitle;         // 창 제목
    bool m_initialized;                 // 초기화 상태
    bool m_vsyncEnabled;               // V-Sync 활성화 상태

public:
    /**
     * @brief 생성자
     */
    OpenGLBackend();
    
    /**
     * @brief 소멸자
     */
    ~OpenGLBackend() override;

    // IGraphicsBackend 인터페이스 구현
    bool initialize(int width, int height, const std::string& title) override;
    void shutdown() override;
    void newFrame() override;
    void endFrame() override;
    void setRenderTarget() override;
    void clear(float r, float g, float b, float a = 1.0f) override;
    bool initializeImGui() override;
    void shutdownImGui() override;
    bool shouldClose() const override;
    void pollEvents() override;
    void swapBuffers() override;
    GraphicsBackendType getType() const override;
    std::string getBackendInfo() const override;
    bool isInitialized() const override;
    void getWindowSize(int* width, int* height) const override;
    void getFramebufferSize(int* width, int* height) const override;
    void setVSync(bool enabled) override;
    void setWindowTitle(const std::string& title) override;
    void* getNativeWindowHandle() const override;
    void* getNativeContext() const override;

private:
    /**
     * @brief OpenGL 버전 설정
     */
    void setupOpenGLVersion();
    
    /**
     * @brief OpenGL 기본 상태 설정
     */
    void setupOpenGLState();
    
    /**
     * @brief 에러 콜백 함수
     * @param error 에러 코드
     * @param description 에러 설명
     */
    static void errorCallback(int error, const char* description);
    
    /**
     * @brief 창 크기 변경 콜백
     * @param window GLFW 윈도우
     * @param width 새 너비
     * @param height 새 높이
     */
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    
    /**
     * @brief OpenGL 에러 확인
     * @param operation 에러 발생 위치 설명
     * @return 에러 발생 여부
     */
    bool checkGLError(const std::string& operation) const;
};
