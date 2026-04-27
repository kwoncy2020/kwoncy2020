module;

// Segmentation functionality implementation for ScrolledImageWindow
#include <wx/wx.h>
#include <opencv2/opencv.hpp>

// These methods should be added to ScrolledImageWindow class implementation
module ScrolledImageWindow;

cv::Mat ScrolledImageWindow::InitializeSegmentationMask(const cv::Mat& m_originalImage) {
    m_segmentationMask = cv::Mat(m_originalImage.size(), CV_8UC1, cv::Scalar(0));
    return m_segmentationMask;
}

void ScrolledImageWindow::SetSegmentationMode(bool enable) {
    m_isSegmentationMode = enable;
    // TODO: should notify to controller
    Refresh();
}

bool ScrolledImageWindow::GetSegmentationMode() const {
    return m_isSegmentationMode;
}

void ScrolledImageWindow::ClearSegmentation() {
    m_segmentationMask = InitializeSegmentationMask(m_originalImage);
    Refresh();
}


void ScrolledImageWindow::SetWXBitmapPixelAt(wxBitmap& bmp, int x, int y) {
    // warning! m_originalImage should be in BGR format
    cv::Vec3b pixel = m_originalImage.at<cv::Vec3b>(y, x);
    unsigned char b = std::min(pixel[0]+m_pixelWeightB, 255);
    unsigned char g = std::min(pixel[1]+m_pixelWeightG, 255);
    unsigned char r = std::min(pixel[2]+m_pixelWeightR, 255);
    
    // Set pixel at specified position
    m_controller->SetWXBitmapPixelAt(bmp, x, y, r, g, b);
    // no require notify
}

void ScrolledImageWindow::UnSetWXBitmapPixelAt(wxBitmap& bmpOut, wxBitmap& bmpOrig,int x, int y) {
    // Unset pixel at specified position
    m_controller->UnSetWXBitmapPixelAt(bmpOut, bmpOrig, x, y);
    // no require notify
}

void ScrolledImageWindow::PerformMaskSegmentation(int x, int y) {
    m_segmentationMask.at<uchar>(y, x) = 255;
    return;
}

void ScrolledImageWindow::ApplyPixelOverlay(int x, int y) {
    // TODO:
    // if (m_segmentationImageRGB_Bitmap.empty() || m_segmentationImageRGB_Bitmap.empty()) {
    //     return;
    // }
    // cv::Vec3b pixel = m_segmentationImageRGB_Bitmap.at<cv::Vec3b>(y, x);
    // m_segmentationImageRGB_Bitmap.at<cv::Vec3b>(y, x) = cv::Vec3b(pixel[0], pixel[1]+127, pixel[2]+40);
    
    // Refresh();
}

// void ScrolledImageWindow::PerformPixelSegmentation(int imageX, int imageY, bool reaquestProcessor = false) {
//     if (!m_isSegmentationMode || m_segmentationImageRGB_Bitmap.empty() || m_segmentationMask.empty()) {
//         return;
//     }
    
//     // Check bounds
//     if (imageX < 0 || imageY < 0 || imageX >= m_segmentationImageRGB_Bitmap.cols || imageY >= m_segmentationImageRGB_Bitmap.rows) {
//         return;
//     }
    
//     if (reaquestProcessor){
//         // Perform segmentation using ImageProcessor
//         m_segmentationMask = ImageProcessor::PerformPixelSegmentation(m_segmentationImageRGB_Bitmap, m_segmentationMask, imageX, imageY, ImageData::ImageFormat::RGB);
//     }
    
//     // Update display
//     Refresh();
// }
