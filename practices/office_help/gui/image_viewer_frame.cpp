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
    Bind(wxEVT_PAINT, &ImageViewerFrame::OnPaint, this);
    
    // Set Korean-friendly font
    wxFont koreanFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Malgun Gothic"));
    SetFont(koreanFont);
    
    // Set initial window size
    SetClientSize(640, 480);
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

void ImageViewerFrame::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    
    if (!m_raw.empty()) {
        // Convert OpenCV Mat to wxImage for display
        cv::Mat rgbMat;
        if (m_raw.channels() == 3) {
            cv::cvtColor(m_raw, rgbMat, cv::COLOR_BGR2RGB);
        } else if (m_raw.channels() == 1) {
            cv::cvtColor(m_raw, rgbMat, cv::COLOR_GRAY2RGB);
        } else {
            rgbMat = m_raw.clone();
        }
        
        wxImage wxImg(rgbMat.cols, rgbMat.rows, rgbMat.data, true);
        wxBitmap bitmap(wxImg);
        
        // Get client area and scale image to fit
        wxSize clientSize = GetClientSize();
        double scaleX = static_cast<double>(clientSize.GetWidth()) / bitmap.GetWidth();
        double scaleY = static_cast<double>(clientSize.GetHeight()) / bitmap.GetHeight();
        double scale = std::min(scaleX, scaleY);
        
        int scaledWidth = static_cast<int>(bitmap.GetWidth() * scale);
        int scaledHeight = static_cast<int>(bitmap.GetHeight() * scale);
        
        wxBitmap scaledBitmap = bitmap.ConvertToImage().Scale(scaledWidth, scaledHeight, wxIMAGE_QUALITY_HIGH);
        
        // Center the image
        int x = (clientSize.GetWidth() - scaledWidth) / 2;
        int y = (clientSize.GetHeight() - scaledHeight) / 2;
        
        dc.DrawBitmap(scaledBitmap, x, y);
    } else {
        // Draw placeholder text when no image is loaded
        dc.SetBrush(wxBrush(GetBackgroundColour()));
        dc.Clear();
        
        dc.DrawText("no image", wxPoint(10, 10));
    }
}