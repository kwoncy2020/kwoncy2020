#include "image/ImageViewer.h"
#include "backends/OpenGLImageLoader.h"
#include "imgui.h"
#include <iostream>
#include <algorithm>

// ImVec2 헤더 포함 (ImGui 내부에 있지만 명시적 포함이 필요할 수 있음)
#include <cmath>

ImageViewer::ImageViewer(const std::string& title) 
    : m_windowTitle(title) {
    m_frameBuffer = std::make_unique<FrameBuffer>(title);
}

ImageViewer::~ImageViewer() {
    cleanup();
}

void ImageViewer::setImage(std::shared_ptr<Image> image) {
    m_currentImage = image;
    
    if (image && image->isLoaded()) {
        // 프레임 버퍼에 이미지 데이터 설정
        if (!image->getCpuData().empty()) {
            m_frameBuffer->updateData(image->getCpuData());
        }
        
        // 뷰포트 업데이트
        updateViewport();
    }
}

void ImageViewer::setImageFromMat(const cv::Mat& mat, const std::string& name) {
    auto image = std::make_shared<Image>(mat, name);
    setImage(image);
}

void ImageViewer::render(bool showWindow) {
    if (!m_currentImage || !m_currentImage->isLoaded()) {
        if (showWindow && m_windowOpen) {
            ImGui::Begin(m_windowTitle.c_str(), &m_windowOpen);
            ImGui::Text("No image loaded");
            ImGui::End();
        }
        return;
    }
    
    if (showWindow) {
        ImGui::Begin(m_windowTitle.c_str(), &m_windowOpen);
    }
    
    // 이미지 컨트롤 렌더링
    renderImageControls();
    
    ImGui::Separator();
    
    // 이미지 디스플레이 렌더링
    renderImageDisplay();
    
    // 이미지 정보 렌더링
    if (m_showImageInfo) {
        renderImageInfo();
    }
    
    // 픽셀 정보 렌더링
    if (m_showPixelInfo && isMouseOverImage()) {
        renderPixelInfo();
    }
    
    // 마우스 이벤트 처리
    handleMouseEvents();
    
    // 키보드 이벤트 처리
    handleKeyboardEvents();
    
    if (showWindow) {
        ImGui::End();
    }
}

void ImageViewer::setViewMode(ViewMode mode) {
    m_viewMode = mode;
    
    switch (mode) {
        case ViewMode::FitToWindow:
            fitToWindow();
            break;
        case ViewMode::ActualSize:
            actualSize();
            break;
        case ViewMode::Zoom:
            // 현재 확대/축소 비율 유지
            break;
    }
    
    updateViewport();
}

void ImageViewer::setScale(float scale) {
    m_scale = std::max(0.1f, std::min(10.0f, scale));
    updateViewport();
}

void ImageViewer::setScrollOffset(const ImVec2& offset) {
    m_scrollOffset = offset;
}

void ImageViewer::updateViewport() {
    if (!m_currentImage || !m_currentImage->isLoaded()) {
        return;
    }
    
    calculateImageLayout();
}

ImVec2 ImageViewer::screenToImage(const ImVec2& screenPos) const {
    if (!m_currentImage || !m_currentImage->isLoaded()) {
        return ImVec2(-1, -1);
    }
    
    ImVec2 relativePos = ImVec2(screenPos.x - m_imagePos.x, screenPos.y - m_imagePos.y);
    
    return ImVec2(
        static_cast<int>(relativePos.x / m_scale),
        static_cast<int>(relativePos.y / m_scale)
    );
}

ImVec2 ImageViewer::imageToScreen(const ImVec2& imagePos) const {
    if (!m_currentImage || !m_currentImage->isLoaded()) {
        return ImVec2(0, 0);
    }
    
    return ImVec2(
        m_imagePos.x + imagePos.x * m_scale,
        m_imagePos.y + imagePos.y * m_scale
    );
}

bool ImageViewer::isMouseOverImage() const {
    if (!m_currentImage || !m_currentImage->isLoaded()) {
        return false;
    }
    
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 imageMin = m_imagePos;
    ImVec2 imageMax = ImVec2(m_imagePos.x + m_imageSize.x, m_imagePos.y + m_imageSize.y);
    
    return (mousePos.x >= imageMin.x && mousePos.x <= imageMax.x &&
            mousePos.y >= imageMin.y && mousePos.y <= imageMax.y);
}

ImVec2 ImageViewer::getCurrentPixelCoords() const {
    if (!isMouseOverImage()) {
        return ImVec2(-1, -1);
    }
    
    ImVec2 mousePos = ImGui::GetMousePos();
    return screenToImage(mousePos);
}

void ImageViewer::fitToWindow() {
    if (!m_currentImage || !m_currentImage->isLoaded()) {
        return;
    }
    
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    const ImageData& data = m_currentImage->getData();
    
    if (data.width <= 0 || data.height <= 0) {
        return;
    }
    
    float scaleX = windowSize.x / data.width;
    float scaleY = windowSize.y / data.height;
    m_scale = std::min(scaleX, scaleY);
    
    // 스크롤 오프셋 초기화
    m_scrollOffset = ImVec2(0, 0);
}

void ImageViewer::actualSize() {
    m_scale = 1.0f;
    updateViewport();
}

void ImageViewer::zoom(float delta, const ImVec2& center) {
    if (!m_enableZoom) {
        return;
    }
    
    float scaleFactor = 1.0f + delta * 0.1f;
    float newScale = m_scale * scaleFactor;
    
    // 확대/축소 한계 적용
    newScale = std::max(0.1f, std::min(10.0f, newScale));
    
    if (newScale != m_scale) {
        ImVec2 zoomCenter = (center.x >= 0 && center.y >= 0) ? center : 
                           ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 
                                 ImGui::GetContentRegionAvail().y * 0.5f);
        
        // 스크롤 오프셋 조정 (확대/축소 중심점 유지)
        m_scrollOffset.x = zoomCenter.x - (zoomCenter.x - m_scrollOffset.x) * (newScale / m_scale);
        m_scrollOffset.y = zoomCenter.y - (zoomCenter.y - m_scrollOffset.y) * (newScale / m_scale);
        
        m_scale = newScale;
        updateViewport();
    }
}

void ImageViewer::cleanup() {
    m_currentImage.reset();
    if (m_frameBuffer) {
        m_frameBuffer->cleanup();
    }
}

// Private methods

void ImageViewer::renderImageControls() {
    if (ImGui::Button("Fit to Window")) {
        setViewMode(ViewMode::FitToWindow);
    }
    ImGui::SameLine();
    if (ImGui::Button("Actual Size")) {
        setViewMode(ViewMode::ActualSize);
    }
    
    // 확대/축소 슬라이더
    ImGui::SliderFloat("Zoom", &m_scale, 0.1f, 10.0f, "%.2f");
    
    // 프리뷰/원본 전환
    bool usePreview = m_usePreview;
    if (ImGui::Checkbox("Use Preview", &usePreview)) {
        m_usePreview = usePreview;
        updateViewport();
    }
    
    // 정보 표시 토글
    ImGui::Checkbox("Show Image Info", &m_showImageInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Show Pixel Info", &m_showPixelInfo);
}

void ImageViewer::renderImageDisplay() {
    if (!m_currentImage || !m_currentImage->isLoaded()) {
        ImGui::Text("No image loaded");
        return;
    }
    
    void* textureId = getSelectedTexture();
    if (!textureId) {
        ImGui::Text("No texture available");
        return;
    }
    
    // 이미지 표시
    ImGui::Image(textureId, m_imageSize);
    
    // 이미지 영역 저장
    m_imagePos = ImGui::GetItemRectMin();
    m_imageSize = ImGui::GetItemRectSize();
}

void ImageViewer::renderPixelInfo() {
    ImVec2 pixelCoords = getCurrentPixelCoords();
    
    if (pixelCoords.x >= 0 && pixelCoords.y >= 0) {
        const ImageData& data = m_currentImage->getData();
        
        if (pixelCoords.x < data.width && pixelCoords.y < data.height) {
            ImGui::BeginTooltip();
            ImGui::Text("Pixel: (%d, %d)", static_cast<int>(pixelCoords.x), static_cast<int>(pixelCoords.y));
            
            // 픽셀 값 표시 (CPU 데이터가 있는 경우)
            const cv::Mat& cpuData = m_currentImage->getCpuData();
            if (!cpuData.empty() && 
                pixelCoords.x < cpuData.cols && 
                pixelCoords.y < cpuData.rows) {
                
                cv::Vec3b pixel = cpuData.at<cv::Vec3b>(static_cast<int>(pixelCoords.y), 
                                                      static_cast<int>(pixelCoords.x));
                ImGui::Text("RGB: (%d, %d, %d)", pixel[2], pixel[1], pixel[0]); // BGR -> RGB
            }
            
            ImGui::EndTooltip();
        }
    }
}

void ImageViewer::renderImageInfo() {
    if (m_currentImage && m_currentImage->isLoaded()) {
        const ImageData& data = m_currentImage->getData();
        ImGui::Text("Size: %d x %d", data.width, data.height);
        ImGui::Text("Channels: %d", data.channels);
        ImGui::Text("Scale: %.2f", m_scale);
        ImGui::Text("Type: %s", data.isStreaming ? "Streaming" : "Static");
        
        if (!m_currentImage->getName().empty()) {
            ImGui::Text("Name: %s", m_currentImage->getName().c_str());
        }
    }
}

void ImageViewer::handleMouseEvents() {
    if (!m_enablePanning) {
        return;
    }
    
    ImVec2 mousePos = ImGui::GetMousePos();
    
    // 마우스 휠 확대/축소
    if (isMouseOverImage() && ImGui::GetIO().MouseWheel != 0.0f) {
        zoom(ImGui::GetIO().MouseWheel, mousePos);
        
        if (m_onMouseScroll) {
            m_onMouseScroll(ImGui::GetIO().MouseWheel);
        }
    }
    
    // 마우스 드래그 패닝
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && isMouseOverImage()) {
        m_isDragging = true;
        m_dragStartPos = mousePos;
        
        if (m_onMouseClick) {
            ImVec2 pixelCoords = getCurrentPixelCoords();
            m_onMouseClick(static_cast<int>(pixelCoords.x), static_cast<int>(pixelCoords.y), 0);
        }
    }
    
    if (m_isDragging && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 dragDelta = ImVec2(mousePos.x - m_dragStartPos.x, mousePos.y - m_dragStartPos.y);
        m_scrollOffset.x += dragDelta.x;
        m_scrollOffset.y += dragDelta.y;
        m_dragStartPos = mousePos;
        updateViewport();
    }
    
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_isDragging = false;
    }
    
    // 마우스 이동
    if (isMouseOverImage()) {
        ImVec2 pixelCoords = getCurrentPixelCoords();
        if (m_onMouseMove && pixelCoords.x >= 0 && pixelCoords.y >= 0) {
            m_onMouseMove(static_cast<int>(pixelCoords.x), static_cast<int>(pixelCoords.y));
        }
    }
}

void ImageViewer::handleKeyboardEvents() {
    // 키보드 단축키 처리
    ImGuiIO& io = ImGui::GetIO();
    
    if (io.KeysDown[ImGuiKey_Space]) {
        setViewMode(ViewMode::FitToWindow);
    }
    
    if (io.KeysDown[ImGuiKey_1]) {
        setScale(1.0f);
    }
    
    if (io.KeysDown[ImGuiKey_Plus] || io.KeysDown[ImGuiKey_Equal]) {
        zoom(0.1f);
    }
    
    if (io.KeysDown[ImGuiKey_Minus]) {
        zoom(-0.1f);
    }
}

void ImageViewer::calculateImageLayout() {
    if (!m_currentImage || !m_currentImage->isLoaded()) {
        return;
    }
    
    const ImageData& data = m_currentImage->getData();
    
    if (m_viewMode == ViewMode::FitToWindow) {
        ImVec2 windowSize = ImGui::GetContentRegionAvail();
        float scaleX = windowSize.x / data.width;
        float scaleY = windowSize.y / data.height;
        m_scale = std::min(scaleX, scaleY);
    }
    
    // 표시 크기 계산
    m_imageSize.x = data.width * m_scale;
    m_imageSize.y = data.height * m_scale;
    
    // 위치 계산 (스크롤 오프셋 적용)
    ImVec2 windowPos = ImGui::GetCursorScreenPos();
    m_imagePos.x = windowPos.x - m_scrollOffset.x;
    m_imagePos.y = windowPos.y - m_scrollOffset.y;
}

void* ImageViewer::getSelectedTexture() const {
    if (!m_currentImage || !m_currentImage->isLoaded()) {
        return nullptr;
    }
    
    const ImageData& data = m_currentImage->getData();
    
    if (m_usePreview && data.previewTextureId) {
        return data.previewTextureId;
    }
    
    return data.textureId;
}
