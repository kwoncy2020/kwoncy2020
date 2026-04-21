module;
#include <wx/wx.h>
#include <opencv2/opencv.hpp>

export module ZoomFrame;

export class ZoomFrame : public wxFrame {
    public:
        ZoomFrame(wxWindow* parent);
        void UpdateDisplay(const cv::Mat& mat);
    private:
        wxBitmap m_bitmap;
        void OnPaint(wxPaintEvent& evt);
};
