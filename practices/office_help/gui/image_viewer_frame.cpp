module;
#include <wx/wx.h>
#include <wx/notebook.h>
#include <opencv2/opencv.hpp> 

module ImageViewerFrame;

import WindowFactory;
import ImageProcessor;
import ZoomFrame;

ImageViewerFrame::ImageViewerFrame(wxWindow* parent, const cv::Mat& frame) : wxFrame(parent, wxID_ANY, "Image Viewer"), m_raw(frame) {
    // 팩토리를 통해 자식(돋보기) 생성
    m_zoomWindow = static_cast<ZoomFrame*>(WindowManager::Get().CreateWindow(WindowType::TYPE_ZOOM_POPUP, this));
    
    Bind(wxEVT_MOTION, &ImageViewerFrame::OnMouseMove, this);
}

void ImageViewerFrame::OnMouseMove(wxMouseEvent& event) {
    wxPoint pos = event.GetPosition();
    
    // [비즈니스 로직 호출] 확대 이미지 얻기
    cv::Mat zoomed = ImageProcessor::GetZoomedROI(m_raw, cv::Point(pos.x, pos.y), 4.0, cv::Size(250, 250));
    
    if (!zoomed.empty()) {
        m_zoomWindow->UpdateDisplay(zoomed);
        m_zoomWindow->SetPosition(wxGetMousePosition() + wxPoint(15, 15));
        if (!m_zoomWindow->IsShown()) m_zoomWindow->Show();
    }
}