module;
#include <wx/wx.h>
#include <wx/bitmap.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <wx/rawbmp.h>

module ImageProcessor;

cv::Mat ImageProcessor::GetZoomedROI(const cv::Mat& src, cv::Point center, double zoom, cv::Size outputSize) {
    if (src.empty()) return cv::Mat();

    int w = outputSize.width / zoom;
    int h = outputSize.height / zoom;

    cv::Rect roi(center.x - w / 2, center.y - h / 2, w, h);
    
    // Clamp ROI to image boundaries
    roi &= cv::Rect(0, 0, src.cols, src.rows);
    if (roi.width <= 0 || roi.height <= 0) return cv::Mat();

    cv::Mat cropped = src(roi).clone();
    cv::resize(cropped, cropped, outputSize, 0, 0, cv::INTER_NEAREST); // No interpolation for pixel observation
    return cropped;
}

cv::Mat ImageProcessor::LoadImageFromFile(const std::string& filePath) {
    return cv::imread(filePath);
}

cv::Mat ImageProcessor::CropImage(const cv::Mat& src, int x, int y, int width, int height) {
    if (src.empty() || !ValidateCropRegion(src, x, y, width, height)) {
        return cv::Mat();
    }
    
    cv::Rect cropRect(x, y, width, height);
    return src(cropRect).clone();
}

bool ImageProcessor::ValidateCropRegion(const cv::Mat& src, int x, int y, int width, int height) {
    if (src.empty()) return false;
    
    // Check if coordinates are valid
    if (x < 0 || y < 0 || width <= 0 || height <= 0) {
        return false;
    }
    
    // Check if region is within image boundaries
    if (x + width > src.cols || y + height > src.rows) {
        return false;
    }
    
    return true;
}

wxBitmap ImageProcessor::ConvertMatToWXBitmap(const cv::Mat& mat) {
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

cv::Mat ImageProcessor::DrawBoundingBoxesOnImage(const cv::Mat& image, const std::vector<BoundingBox>& boxes, int selectedIndex) {
    cv::Mat result = image.clone();
    
    for (size_t i = 0; i < boxes.size(); ++i) {
        const auto& box = boxes[i];
        // Validate bounding box coordinates
        if (box.x < 0 || box.y < 0 || box.x + box.width > image.cols || 
            box.y + box.height > image.rows || box.width <= 0 || box.height <= 0) {
            continue; // Skip invalid boxes
        }
        
        // Choose color and thickness based on selection
        cv::Scalar color;
        int thickness;
        if (selectedIndex >= 0 && i == static_cast<size_t>(selectedIndex)) {
            // Selected box - red color, thicker line
            color = cv::Scalar(0, 0, 255); // Red (BGR format)
            thickness = 3; // 3px thickness for selected box
        } else {
            // Normal box - green color, normal line
            color = cv::Scalar(0, 255, 0); // Green (BGR format)
            thickness = 2; // 2px thickness for normal box
        }
        
        cv::rectangle(result, 
                     cv::Point(box.x, box.y), 
                     cv::Point(box.x + box.width, box.y + box.height), 
                     color, thickness);
    }
    
    return result;
}



// Segmentation functionality implementation
cv::Mat ImageProcessor::InitializeSegmentationMask(const cv::Mat& image) {
    if (image.empty()) {
        return cv::Mat();
    }
    
    // Create a black mask (all zeros) - same size as input image
    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);
    return mask;
}

void ImageProcessor::SetWXBitmapPixelAt(wxBitmap& bmp, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    wxNativePixelData data(bmp);
    if (!data) return;

    wxNativePixelData::Iterator p(data);
    
    // OpenCV의 at(y, x)처럼 특정 좌표로 바로 이동
    p.MoveTo(data, x, y); 

    // 해당 위치의 픽셀 수정
    p.Red()   = r;
    p.Green() = g;
    p.Blue()  = b;
    
    return;
}

void ImageProcessor::UnSetWXBitmapPixelAt(wxBitmap& bmpOut, wxBitmap& bmpOrig, int x, int y) {
    wxNativePixelData dataOut(bmpOut);
    wxNativePixelData dataOrig(bmpOrig);
    if (!dataOut || !dataOrig) return;

    wxNativePixelData::Iterator pOut(dataOut);
    wxNativePixelData::Iterator pOrig(dataOrig);
    
    // OpenCV의 at(y, x)처럼 특정 좌표로 바로 이동
    pOut.MoveTo(dataOut, x, y); 
    pOrig.MoveTo(dataOrig, x, y);

    // 해당 위치의 픽셀 수정
    pOut.Red()   = pOrig.Red();
    pOut.Green() = pOrig.Green();
    pOut.Blue()  = pOrig.Blue();
    
    return;
}

// should be recheck.
cv::Mat ImageProcessor::PerformPixelSegmentation(const cv::Mat& image, const cv::Mat& currentMask, int x, int y) {
    if (image.empty() || currentMask.empty()) {
        return cv::Mat();
    }
    
    // Check bounds
    if (x < 0 || y < 0 || x >= image.cols || y >= image.rows) {
        return currentMask.clone();
    }
    
    cv::Mat newMask = currentMask.clone();
    
    // Simple flood fill segmentation starting from the clicked pixel
    cv::Point seed(x, y);
    cv::Scalar newVal(255); // White for segmented area
    
    // Get the color at the clicked pixel
    cv::Vec3b pixelColor = image.at<cv::Vec3b>(y, x);
    cv::Scalar seedColor(pixelColor[0], pixelColor[1], pixelColor[2]);
    
    // Define color tolerance for segmentation (can be adjusted)
    cv::Scalar lowerDiff(30, 30, 30);
    cv::Scalar upperDiff(30, 30, 30);
    
    // Perform flood fill
    cv::floodFill(newMask, seed, newVal, 0, lowerDiff, upperDiff, cv::FLOODFILL_FIXED_RANGE);
    
    return newMask;
}

// TODO: should be checked.
cv::Mat ImageProcessor::ApplySegmentationOverlay(const cv::Mat& image, const cv::Mat& mask, ImageData::ImageFormat inputImageFormat) {
    if (image.empty() || mask.empty()) {
        return image.clone();
    }
    
    cv::Mat result = image.clone();
    int c1,c2,c3;
    // Currently, same green tint will be applied to all format
    if (inputImageFormat == ImageData::ImageFormat::BGR) {
        c1 = 0; c2 = 255; c3 = 0;
    } else {
        c1 = 0; c2 = 255; c3 = 0;
    }

    // Apply colored overlay to segmented areas
    for (int y = 0; y < result.rows; y++) {
        for (int x = 0; x < result.cols; x++) {
            if (mask.at<uchar>(y, x) > 0) {
                // Apply semi-transparent colored overlay
                cv::Vec3b& pixel = result.at<cv::Vec3b>(y, x);
                pixel[0] = (pixel[0] * 0.5 + c1 * 0.5); // Blue tint
                pixel[1] = (pixel[1] * 0.5 + c2 * 0.5); // Green tint
                pixel[2] = (pixel[2] * 0.5 + c3 * 0.5); // Red tint
            }
        }
    }
    
    return result;
}




cv::Mat ImageProcessor::ConvertMatColorSpace(const cv::Mat& image, ImageData::ImageFormat currentFormat, ImageData::ImageFormat targetFormat) {
    // If formats are the same, return clone for safety
    if (currentFormat == targetFormat) {
        return image.clone();
    }
    
    cv::Mat converted;
    
    // Convert from current format to target format
    switch (currentFormat) {
        case ImageData::ImageFormat::BGR:
            switch (targetFormat) {
                case ImageData::ImageFormat::RGB:
                    cv::cvtColor(image, converted, cv::COLOR_BGR2RGB);
                    break;
                case ImageData::ImageFormat::GRAYSCALE:
                    cv::cvtColor(image, converted, cv::COLOR_BGR2GRAY);
                    break;
                case ImageData::ImageFormat::BGRA:
                    cv::cvtColor(image, converted, cv::COLOR_BGR2BGRA);
                    break;
                case ImageData::ImageFormat::RGBA:
                    cv::cvtColor(image, converted, cv::COLOR_BGR2RGBA);
                    break;
                default:
                    converted = image.clone();
                    break;
            }
            break;
            
        case ImageData::ImageFormat::RGB:
            switch (targetFormat) {
                case ImageData::ImageFormat::BGR:
                    cv::cvtColor(image, converted, cv::COLOR_RGB2BGR);
                    break;
                case ImageData::ImageFormat::GRAYSCALE:
                    cv::cvtColor(image, converted, cv::COLOR_RGB2GRAY);
                    break;
                case ImageData::ImageFormat::RGBA:
                    cv::cvtColor(image, converted, cv::COLOR_RGB2RGBA);
                    break;
                case ImageData::ImageFormat::BGRA:
                    cv::cvtColor(image, converted, cv::COLOR_RGB2BGRA);
                    break;
                default:
                    converted = image.clone();
                    break;
            }
            break;
            
        case ImageData::ImageFormat::GRAYSCALE:
            switch (targetFormat) {
                case ImageData::ImageFormat::BGR:
                    cv::cvtColor(image, converted, cv::COLOR_GRAY2BGR);
                    break;
                case ImageData::ImageFormat::RGB:
                    cv::cvtColor(image, converted, cv::COLOR_GRAY2RGB);
                    break;
                case ImageData::ImageFormat::BGRA:
                    cv::cvtColor(image, converted, cv::COLOR_GRAY2BGRA);
                    break;
                case ImageData::ImageFormat::RGBA:
                    cv::cvtColor(image, converted, cv::COLOR_GRAY2RGBA);
                    break;
                default:
                    converted = image.clone();
                    break;
            }
            break;
            
        case ImageData::ImageFormat::BGRA:
            switch (targetFormat) {
                case ImageData::ImageFormat::BGR:
                    cv::cvtColor(image, converted, cv::COLOR_BGRA2BGR);
                    break;
                case ImageData::ImageFormat::RGB:
                    cv::cvtColor(image, converted, cv::COLOR_BGRA2RGB);
                    break;
                case ImageData::ImageFormat::GRAYSCALE:
                    cv::cvtColor(image, converted, cv::COLOR_BGRA2GRAY);
                    break;
                case ImageData::ImageFormat::RGBA:
                    // BGRA to RGBA requires channel reordering
                    cv::cvtColor(image, converted, cv::COLOR_BGRA2RGBA);
                    break;
                default:
                    converted = image.clone();
                    break;
            }
            break;
            
        case ImageData::ImageFormat::RGBA:
            switch (targetFormat) {
                case ImageData::ImageFormat::BGR:
                    cv::cvtColor(image, converted, cv::COLOR_RGBA2BGR);
                    break;
                case ImageData::ImageFormat::RGB:
                    cv::cvtColor(image, converted, cv::COLOR_RGBA2RGB);
                    break;
                case ImageData::ImageFormat::GRAYSCALE:
                    cv::cvtColor(image, converted, cv::COLOR_RGBA2GRAY);
                    break;
                case ImageData::ImageFormat::BGRA:
                    // RGBA to BGRA requires channel reordering
                    cv::cvtColor(image, converted, cv::COLOR_RGBA2BGRA);
                    break;
                default:
                    converted = image.clone();
                    break;
            }
            break;
            
        default:
            converted = image.clone();
            break;
    }
    
    return converted;
}
