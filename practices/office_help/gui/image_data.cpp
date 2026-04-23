module;
#include <chrono>
#include <iomanip>
#include <sstream>
#include <opencv2/opencv.hpp>

module ImageData;

// ImageData class implementation
ImageData::ImageData(const cv::Mat& image, ImageFormat format) {
    SetImage(image);
    m_imageFormatEnum = format;
    m_creationTime = GetCurrentTimestamp();
}

ImageData::ImageData(const cv::Mat& image, const std::string& filename, ImageFormat format) {
    SetImage(image);
    m_filename = filename;
    m_imageFormatEnum = format;
    m_creationTime = GetCurrentTimestamp();
}

void ImageData::SetImage(const cv::Mat& image) {
    m_image = image.clone();
    if (!m_image.empty()) {
        UpdateImageMetadata();
    }
}

void ImageData::SetMetadata(const std::string& filename, const std::string& timestamp) {
    m_filename = filename;
    m_creationTime = timestamp;
}

void ImageData::AddBoundingBox(const BoundingBox& box) {
    m_boundingBoxes.push_back(box);
}

void ImageData::RemoveBoundingBox(int index) {
    if (index >= 0 && index < m_boundingBoxes.size()) {
        m_boundingBoxes.erase(m_boundingBoxes.begin() + index);
    }
}

void ImageData::RemoveBoundingBoxAt(int x, int y) {
    int index = FindBoundingBoxAt(x, y);
    if (index != -1) {
        RemoveBoundingBox(index);
    }
}

int ImageData::FindBoundingBoxAt(int x, int y) const {
    // reverse order to find the topmost box first
    for (size_t i = m_boundingBoxes.size()-1; i>=0 ; --i) {
        if (m_boundingBoxes[i].Contains(x, y)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void ImageData::ClearBoundingBoxes() {
    m_boundingBoxes.clear();
}


void ImageData::UpdateImageMetadata() {
    if (m_image.empty()) {
        m_channels = 0;
        m_depth = 0;
        m_imageSize = 0;
        m_imageFormat = "Empty";
        return;
    }
    
    // Update channel information
    m_channels = m_image.channels();
    
    // Update depth information
    m_depth = m_image.depth();
    
    // Calculate image size in bytes
    m_imageSize = m_image.total() * m_image.elemSize();
    
    // Create format description based on custom ImageFormat enum
    m_imageFormat = GetDepthString(m_depth);
    switch (m_imageFormatEnum) {
        case ImageFormat::GRAYSCALE:
            m_imageFormat += " (Grayscale)";
            break;
        case ImageFormat::RGB:
            m_imageFormat += " (RGB)";
            break;
        case ImageFormat::BGR:
            m_imageFormat += " (BGR)";
            break;
        case ImageFormat::RGBA:
            m_imageFormat += " (RGBA)";
            break;
        case ImageFormat::BGRA:
            m_imageFormat += " (BGRA)";
            break;
        default:
            m_imageFormat += " (" + std::to_string(m_channels) + " channels)";
            break;
    }
}

std::string ImageData::GetImageFormatString() const {
    switch (m_imageFormatEnum) {
        case ImageFormat::GRAYSCALE:
            return "Grayscale";
        case ImageFormat::RGB:
            return "RGB";
        case ImageFormat::BGR:
            return "BGR";
        case ImageFormat::RGBA:
            return "RGBA";
        case ImageFormat::BGRA:
            return "BGRA";
        default:
            return "Unknown";
    }
}

std::string ImageData::GetDepthString(int depth) const {
    switch (depth) {
        case CV_8U: return "8U";
        case CV_8S: return "8S";
        case CV_16U: return "16U";
        case CV_16S: return "16S";
        case CV_32S: return "32S";
        case CV_32F: return "32F";
        case CV_64F: return "64F";
        case CV_16F: return "16F";
        default: return "Unknown";
    }
}

std::string ImageData::GetCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    // Use thread-safe localtime_s (Windows) or localtime_r (POSIX)
    std::tm tm_info;
#ifdef _WIN32
    localtime_s(&tm_info, &time_t);
#else
    localtime_r(&time_t, &tm_info);
#endif
    
    std::stringstream ss;
    ss << std::put_time(&tm_info, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

bool ImageData::IsPointInBoundingBox(const BoundingBox& box, int x, int y) {
    return (x >= box.x && x <= box.x + box.width &&
            y >= box.y && y <= box.y + box.height);
}
