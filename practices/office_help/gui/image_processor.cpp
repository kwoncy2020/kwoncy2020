module;
#include <wx/wx.h>
#include <wx/bitmap.h>
#include <opencv2/opencv.hpp>
#include <vector>

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

cv::Mat ImageProcessor::DrawBoundingBoxesOnImage(const cv::Mat& image, const std::vector<BoundingBox>& boxes) {
    cv::Mat result = image.clone();
    
    for (const auto& box : boxes) {
        // Validate bounding box coordinates
        if (box.x < 0 || box.y < 0 || box.x + box.width > image.cols || 
            box.y + box.height > image.rows || box.width <= 0 || box.height <= 0) {
            continue; // Skip invalid boxes
        }
        
        // Draw green rectangle for bounding box
        cv::rectangle(result, 
                     cv::Point(box.x, box.y), 
                     cv::Point(box.x + box.width, box.y + box.height), 
                     cv::Scalar(0, 255, 0), 2); // Green color, 2px thickness
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
