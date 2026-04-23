module;
#include <wx/wx.h>
#include <wx/notebook.h>
#include <opencv2/opencv.hpp> 
#include <wx/dcbuffer.h>

module ZoomFrame;

import ImageProcessor;

ZoomFrame::ZoomFrame(wxWindow* parent) : wxFrame(parent, wxID_ANY, "", wxDefaultPosition, wxSize(250, 250), 
    wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP | wxFRAME_TOOL_WINDOW) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &ZoomFrame::OnPaint, this);
}

void ZoomFrame::UpdateDisplay(const cv::Mat& mat) {
    // Mat -> wxBitmap 변환 로직 (실제 구현 시 유틸리티 함수 사용)
    m_bitmap = ImageProcessor::ConvertMatToWXBitmap(mat); 
    Refresh();
}

void ZoomFrame::OnPaint(wxPaintEvent& evt) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    if (m_bitmap.IsOk()) dc.DrawBitmap(m_bitmap, 0, 0);
}
