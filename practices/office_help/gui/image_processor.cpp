module;
#include <wx/wx.h>
#include <wx/bitmap.h>
#include <opencv2/opencv.hpp>

module ImageProcessor;

cv::Mat ImageProcessor::GetZoomedROI(const cv::Mat& src, cv::Point center, double zoom, cv::Size outputSize) {
    if (src.empty()) return cv::Mat();

    int w = outputSize.width / zoom;
    int h = outputSize.height / zoom;

    cv::Rect roi(center.x - w / 2, center.y - h / 2, w, h);
    
    // 이미지 경계 밖으로 나가지 않도록 보정
    roi &= cv::Rect(0, 0, src.cols, src.rows);
    if (roi.width <= 0 || roi.height <= 0) return cv::Mat();

    cv::Mat cropped = src(roi).clone();
    cv::resize(cropped, cropped, outputSize, 0, 0, cv::INTER_NEAREST); // 픽셀 관찰을 위해 보간 없이 확대
    return cropped;
}

// Utility function to convert OpenCV Mat to wxBitmap
wxBitmap ImageProcessor::ConvertMatToBitmap(const cv::Mat& mat) {
    if (mat.empty()) return wxBitmap();
    
    cv::Mat converted;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, converted, cv::COLOR_BGR2RGB);
    } else if (mat.channels() == 4) {
        cv::cvtColor(mat, converted, cv::COLOR_BGRA2RGB);
    } else if (mat.channels() == 1) {
        cv::cvtColor(mat, converted, cv::COLOR_GRAY2RGB);
    } else {
        converted = mat.clone();
    }
    
    wxImage image(converted.cols, converted.rows, converted.data, true);
    return wxBitmap(image);
}