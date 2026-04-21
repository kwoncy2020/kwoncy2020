module;
#include <wx/wx.h>
#include <wx/bitmap.h>
#include <opencv2/opencv.hpp>

export module ImageProcessor;

export class ImageProcessor {
public:
    // 원본에서 특정 지점을 중심으로 확대된 ROI 추출
    static cv::Mat GetZoomedROI(const cv::Mat& src, cv::Point center, double zoom, cv::Size outputSize);

    // OpenCV Mat을 wxBitmap으로 변환
    // static wxBitmap ImageProcessor::ConvertMatToBitmap(const cv::Mat& mat);
    static wxBitmap ConvertMatToBitmap(const cv::Mat& mat);
};
