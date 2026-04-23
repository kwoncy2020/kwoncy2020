module;
#include <wx/wx.h>
#include <wx/bitmap.h>
#include <opencv2/opencv.hpp>
#include <vector>

export module ImageProcessor;

import ImageData;

export class ImageProcessor {
public:
    // Original image zoomed ROI extraction
    static cv::Mat GetZoomedROI(const cv::Mat& src, cv::Point center, double zoom, cv::Size outputSize);

    // OpenCV Mat to wxBitmap conversion
    static wxBitmap ConvertMatToBitmap(const cv::Mat& mat);
    
    // Load image from file path
    static cv::Mat LoadImageFromFile(const std::string& filePath);
    
    // Crop image from specified region
    static cv::Mat CropImage(const cv::Mat& src, int x, int y, int width, int height);
    
    // Validate crop region
    static bool ValidateCropRegion(const cv::Mat& src, int x, int y, int width, int height);
    
    // Bounding box rendering
    static cv::Mat DrawBoundingBoxesOnImage(const cv::Mat& image, const std::vector<BoundingBox>& boxes);

};
