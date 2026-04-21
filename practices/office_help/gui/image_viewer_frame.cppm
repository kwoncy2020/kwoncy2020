module;
#include <wx/wx.h>
#include <opencv2/opencv.hpp>

export module ImageViewerFrame;

import ZoomFrame;

export class ImageViewerFrame : public wxFrame {
    public:
        ImageViewerFrame(wxWindow* parent, const cv::Mat& frame);
    private:
        cv::Mat m_raw;
        ZoomFrame* m_zoomWindow;
        void OnMouseMove(wxMouseEvent& evt);
        void OnPaint(wxPaintEvent& evt);
};
