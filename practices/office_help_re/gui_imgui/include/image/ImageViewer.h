#pragma once

#include "Image.h"
#include "FrameBuffer.h"
#include <memory>
#include <functional>
#include <string>

// Forward declarations for ImGui
struct ImVec2;

/**
 * @brief ImGui 이미지 뷰어 클래스
 * 
 * 이 클래스는 ImGui와 통합된 이미지 뷰어를 제공합니다.
 * 마우스 이벤트 처리, 확대/축소, 스크롤, 픽셀 정보 표시 등
 * 다양한 인터랙티브 기능을 지원합니다.
 * 
 * 주요 기능:
 * - 백엔드 중립적 이미지 표시
 * - 마우스 인터랙션 (확대/축소, 패닝, 스크롤)
 * - 픽셀 정보 표시
 * - 다중 해상도 지원 (프리뷰/원본 전환)
 * - 바운딩박스 오버레이 지원
 */
class ImageViewer {
public:
    /**
     * @brief 뷰어 모드 열거형
     */
    enum class ViewMode {
        FitToWindow,    // 창에 맞춤
        ActualSize,     // 실제 크기
        Zoom            // 사용자 정의 확대/축소
    };

    /**
     * @brief 마우스 이벤트 콜백 타입
     */
    using MouseClickCallback = std::function<void(int x, int y, int button)>;
    using MouseMoveCallback = std::function<void(int x, int y)>;
    using MouseScrollCallback = std::function<void(float delta)>;

private:
    std::shared_ptr<Image> m_currentImage;        // 현재 표시 이미지
    std::unique_ptr<FrameBuffer> m_frameBuffer;   // 프레임 버퍼
    ViewMode m_viewMode = ViewMode::FitToWindow;  // 현재 뷰 모드
    float m_scale = 1.0f;                         // 확대/축소 비율
    ImVec2 m_scrollOffset{0, 0};                 // 스크롤 오프셋
    ImVec2 m_imagePos{0, 0};                     // 이미지 위치
    ImVec2 m_imageSize{0, 0};                    // 표시 이미지 크기
    
    // 인터랙션 상태
    bool m_showPixelInfo = true;                  // 픽셀 정보 표시 여부
    bool m_showImageInfo = true;                  // 이미지 정보 표시 여부
    bool m_enablePanning = true;                  // 패닝 활성화 여부
    bool m_enableZoom = true;                     // 확대/축소 활성화 여부
    bool m_isDragging = false;                    // 드래그 상태
    ImVec2 m_dragStartPos{0, 0};                 // 드래그 시작 위치
    
    // 콜백 함수들
    MouseClickCallback m_onMouseClick;
    MouseMoveCallback m_onMouseMove;
    MouseScrollCallback m_onMouseScroll;
    
    // UI 상태
    std::string m_windowTitle = "Image Viewer";   // 창 제목
    bool m_windowOpen = true;                    // 창 열림 상태
    ImVec2 m_windowSize{800, 600};               // 창 크기
    bool m_usePreview = false;                    // 프리뷰 사용 여부
    int m_maxPreviewSize = 512;                   // 최대 프리뷰 크기

public:
    /**
     * @brief 생성자
     * @param title 창 제목
     */
    explicit ImageViewer(const std::string& title = "Image Viewer");
    
    /**
     * @brief 소멸자
     */
    ~ImageViewer();

    /**
     * @brief 이미지 설정
     * @param image 표시할 이미지
     */
    void setImage(std::shared_ptr<Image> image);
    
    /**
     * @brief OpenCV Mat 데이터로 직접 이미지 설정
     * @param mat OpenCV Mat 데이터
     * @param name 이미지 이름
     */
    void setImageFromMat(const cv::Mat& mat, const std::string& name = "");
    
    /**
     * @brief 메인 렌더링 함수
     * @param showWindow 별도 창으로 표시 여부 (false면 현재 창에 렌더링)
     */
    void render(bool showWindow = true);
    
    /**
     * @brief 뷰 모드 설정
     * @param mode 뷰 모드
     */
    void setViewMode(ViewMode mode);
    
    /**
     * @brief 확대/축소 비율 설정
     * @param scale 확대/축소 비율
     */
    void setScale(float scale);
    
    /**
     * @brief 스크롤 오프셋 설정
     * @param offset 스크롤 오프셋
     */
    void setScrollOffset(const ImVec2& offset);
    
    /**
     * @brief 픽셀 정보 표시 여부 설정
     * @param show 표시 여부
     */
    void setShowPixelInfo(bool show) { m_showPixelInfo = show; }
    
    /**
     * @brief 이미지 정보 표시 여부 설정
     * @param show 표시 여부
     */
    void setShowImageInfo(bool show) { m_showImageInfo = show; }
    
    /**
     * @brief 패닝 활성화 여부 설정
     * @param enable 활성화 여부
     */
    void setEnablePanning(bool enable) { m_enablePanning = enable; }
    
    /**
     * @brief 확대/축소 활성화 여부 설정
     * @param enable 활성화 여부
     */
    void setEnableZoom(bool enable) { m_enableZoom = enable; }
    
    /**
     * @brief 창 제목 설정
     * @param title 창 제목
     */
    void setWindowTitle(const std::string& title) { m_windowTitle = title; }
    
    /**
     * @brief 창 열림 상태 설정
     * @param open 열림 상태
     */
    void setWindowOpen(bool open) { m_windowOpen = open; }
    
    /**
     * @brief 프리뷰 사용 여부 설정
     * @param usePreview 프리뷰 사용 여부
     */
    void setUsePreview(bool usePreview) { m_usePreview = usePreview; }
    
    /**
     * @brief 최대 프리뷰 크기 설정
     * @param size 최대 크기
     */
    void setMaxPreviewSize(int size) { m_maxPreviewSize = size; }
    
    // 콜백 설정 함수들
    void setMouseClickCallback(MouseClickCallback callback) { m_onMouseClick = callback; }
    void setMouseMoveCallback(MouseMoveCallback callback) { m_onMouseMove = callback; }
    void setMouseScrollCallback(MouseScrollCallback callback) { m_onMouseScroll = callback; }
    
    // Getter 함수들
    std::shared_ptr<Image> getCurrentImage() const { return m_currentImage; }
    ViewMode getViewMode() const { return m_viewMode; }
    float getScale() const { return m_scale; }
    bool isWindowOpen() const { return m_windowOpen; }
    const ImVec2& getImageSize() const { return m_imageSize; }
    
    /**
     * @brief 화면 좌표를 이미지 좌표로 변환
     * @param screenPos 화면 좌표
     * @return 이미지 좌표
     */
    ImVec2 screenToImage(const ImVec2& screenPos) const;
    
    /**
     * @brief 이미지 좌표를 화면 좌표로 변환
     * @param imagePos 이미지 좌표
     * @return 화면 좌표
     */
    ImVec2 imageToScreen(const ImVec2& imagePos) const;
    
    /**
     * @brief 이미지 영역에 마우스가 있는지 확인
     * @return 이미지 영역 내 여부
     */
    bool isMouseOverImage() const;
    
    /**
     * @brief 현재 마우스 위치의 픽셀 좌표 가져오기
     * @return 픽셀 좌표 (이미지 밖이면 {-1, -1})
     */
    ImVec2 getCurrentPixelCoords() const;
    
    /**
     * @brief 뷰포트 업데이트
     */
    void updateViewport();
    
    /**
     * @brief 창 크기에 맞게 이미지 크기 조절
     */
    void fitToWindow();
    
    /**
     * @brief 실제 크기로 이미지 표시
     */
    void actualSize();
    
    /**
     * @brief 확대/축소
     * @param delta 확대/축소 양 (양수: 확대, 음수: 축소)
     * @param center 확대/축소 중심점 (기본값: 창 중앙)
     */
    void zoom(float delta, const ImVec2& center = ImVec2(-1, -1));
    
    /**
     * @brief 모든 리소스 정리
     */
    void cleanup();

private:
    /**
     * @brief 이미지 컨트롤 UI 렌더링
     */
    void renderImageControls();
    
    /**
     * @brief 이미지 디스플레이 렌더링
     */
    void renderImageDisplay();
    
    /**
     * @brief 픽셀 정보 렌더링
     */
    void renderPixelInfo();
    
    /**
     * @brief 이미지 정보 렌더링
     */
    void renderImageInfo();
    
    /**
     * @brief 마우스 이벤트 처리
     */
    void handleMouseEvents();
    
    /**
     * @brief 키보드 이벤트 처리
     */
    void handleKeyboardEvents();
    
    /**
     * @brief 이미지 위치와 크기 계산
     */
    void calculateImageLayout();
    
    /**
     * @brief 적절한 텍스처 선택 (원본 또는 프리뷰)
     * @return 텍스처 ID
     */
    void* getSelectedTexture() const;
};
