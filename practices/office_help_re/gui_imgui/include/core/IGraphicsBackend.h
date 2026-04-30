#pragma once

#include <string>
#include <memory>

/**
 * @brief 그래픽 백엔드 타입 열거형
 */
enum class GraphicsBackendType {
    OpenGL,
    Vulkan,
    DirectX,
    Metal
};

/**
 * @brief 그래픽 백엔드 인터페이스
 * 
 * 이 인터페이스는 다양한 그래픽 백엔드(OpenGL, Vulkan, DirectX 등)를
 * 통합된 방식으로 처리하기 위한 공통 API를 제공합니다.
 * 
 * 주요 기능:
 * - 그래픽 컨텍스트 초기화
 * - 창 생성 및 관리
 * - 렌더링 백엔드 설정
 * - ImGui 백엔드 초기화
 */
class IGraphicsBackend {
public:
    virtual ~IGraphicsBackend() = default;

    /**
     * @brief 그래픽 백엔드 초기화
     * @param width 창 너비
     * @param height 창 높이
     * @param title 창 제목
     * @return 성공 여부
     */
    virtual bool initialize(int width, int height, const std::string& title) = 0;

    /**
     * @brief 그래픽 백엔드 종료
     */
    virtual void shutdown() = 0;

    /**
     * @brief 새 프레임 시작
     */
    virtual void newFrame() = 0;

    /**
     * @brief 프레임 렌더링 완료
     */
    virtual void endFrame() = 0;

    /**
     * @brief 렌더링 대상 설정
     */
    virtual void setRenderTarget() = 0;

    /**
     * @brief 화면 지우기
     * @param r 빨간색 (0.0-1.0)
     * @param g 초록색 (0.0-1.0)
     * @param b 파란색 (0.0-1.0)
     * @param a 알파 (0.0-1.0)
     */
    virtual void clear(float r, float g, float b, float a = 1.0f) = 0;

    /**
     * @brief ImGui 백엔드 초기화
     * @return 성공 여부
     */
    virtual bool initializeImGui() = 0;

    /**
     * @brief ImGui 백엔드 종료
     */
    virtual void shutdownImGui() = 0;

    /**
     * @brief 창 닫혔는지 확인
     * @return 닫힘 여부
     */
    virtual bool shouldClose() const = 0;

    /**
     * @brief 이벤트 처리
     */
    virtual void pollEvents() = 0;

    /**
     * @brief 버퍼 스왑
     */
    virtual void swapBuffers() = 0;

    /**
     * @brief 백엔드 타입 가져오기
     * @return 그래픽 백엔드 타입
     */
    virtual GraphicsBackendType getType() const = 0;

    /**
     * @brief 백엔드 정보 가져오기
     * @return 백엔드 정보 문자열
     */
    virtual std::string getBackendInfo() const = 0;

    /**
     * @brief 초기화 상태 확인
     * @return 초기화 완료 여부
     */
    virtual bool isInitialized() const = 0;

    /**
     * @brief 창 크기 가져오기
     * @param width 너비 출력 포인터
     * @param height 높이 출력 포인터
     */
    virtual void getWindowSize(int* width, int* height) const = 0;

    /**
     * @brief 프레임버퍼 크기 가져오기
     * @param width 너비 출력 포인터
     * @param height 높이 출력 포인터
     */
    virtual void getFramebufferSize(int* width, int* height) const = 0;

    /**
     * @brief V-Sync 설정
     * @param enabled 활성화 여부
     */
    virtual void setVSync(bool enabled) = 0;

    /**
     * @brief 창 제목 설정
     * @param title 창 제목
     */
    virtual void setWindowTitle(const std::string& title) = 0;

    /**
     * @brief 네이티브 윈도우 핸들 가져오기 (플랫폼 종속적 기능용)
     * @return 윈도우 핸들
     */
    virtual void* getNativeWindowHandle() const = 0;

    /**
     * @brief 네이티브 컨텍스트 가져오기 (플랫폼 종속적 기능용)
     * @return 그래픽 컨텍스트
     */
    virtual void* getNativeContext() const = 0;
};
