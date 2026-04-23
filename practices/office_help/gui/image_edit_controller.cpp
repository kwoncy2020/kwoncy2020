module;

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <algorithm>
#include <wx/wx.h>
#include <wx/arrstr.h>

module ImageEditController;

import ImageData;
import ImageDataRepository;
import ImageProcessor;

ImageEditController::ImageEditController() 
    : m_repository(nullptr) {
}

ImageEditController::ImageEditController(ImageDataRepository* repository) 
    : m_repository(repository) {
}

std::string ImageEditController::LoadImageFromFile(const std::string& filepath, ImageData::ImageFormat format) {
    if (!IsValidRepository()) {
        return "";
    }
    
    // Check if file exists
    if (!std::filesystem::exists(filepath)) {
        return "";
    }
    
    // Convert ImageFormat to OpenCV flags and load image
    int opencvFlags = cv::IMREAD_COLOR; // Default
    switch (format) {
        case ImageData::ImageFormat::GRAYSCALE:
            opencvFlags = cv::IMREAD_GRAYSCALE;
            break;
        case ImageData::ImageFormat::RGB:
        case ImageData::ImageFormat::BGR:
            opencvFlags = cv::IMREAD_COLOR;
            break;
        case ImageData::ImageFormat::RGBA:
        case ImageData::ImageFormat::BGRA:
            opencvFlags = cv::IMREAD_UNCHANGED;
            break;
        default:
            opencvFlags = cv::IMREAD_COLOR;
            break;
    }
    
    cv::Mat image = cv::imread(filepath, opencvFlags);
    if (image.empty()) {
        return "";
    }
    
    // Generate name from filepath
    std::string name = GenerateImageName(filepath);
    
    // Create image data with format through repository
    return m_repository->CreateImageData(image, name, format);
}

std::string ImageEditController::LoadImageFromMat(const cv::Mat& image, const std::string& name, ImageData::ImageFormat format) {
    if (!IsValidRepository()) {
        return "";
    }
    
    if (image.empty()) {
        return "";
    }
    
    std::string imageName = name.empty() ? "unnamed_image" : name;
    return m_repository->CreateImageData(image, imageName, format);
}

bool ImageEditController::RemoveImage(const std::string& id) {
    if (!IsValidRepository()) {
        return false;
    }
    
    return m_repository->RemoveImageData(id);
}

ImageData* ImageEditController::GetImageData(const std::string& id) {
    if (!IsValidRepository()) {
        return nullptr;
    }
    
    return m_repository->GetImageData(id);
}

cv::Mat ImageEditController::GetImage(const std::string& id) {
    ImageData* imageData = GetImageData(id);
    if (!imageData) {
        return cv::Mat();
    }
    
    return imageData->GetImage();
}

cv::Mat ImageEditController::GetDisplayImage(const std::string& id) {
    ImageData* imageData = GetImageData(id);
    if (!imageData) {
        return cv::Mat();
    }
    
    // Use ImageProcessor to render bounding boxes on the image
    cv::Mat originalImage = imageData->GetImage();
    std::vector<BoundingBox> boxes = imageData->GetBoundingBoxes();
    
    return ImageProcessor::DrawBoundingBoxesOnImage(originalImage, boxes);
}

std::string ImageEditController::GetImageName(const std::string& id) const {
    if (!IsValidRepository()) {
        return "";
    }
    
    return m_repository->GetImageName(id);
}

void ImageEditController::SetImageName(const std::string& id, const std::string& name) {
    if (!IsValidRepository()) {
        return;
    }
    
    m_repository->SetImageName(id, name);
}

bool ImageEditController::HasImage(const std::string& id) const {
    if (!IsValidRepository()) {
        return false;
    }
    
    return m_repository->HasImageData(id);
}

std::vector<std::string> ImageEditController::GetAllImageIds() const {
    if (!IsValidRepository()) {
        return {};
    }
    
    return m_repository->GetAllIds();
}

size_t ImageEditController::GetImageCount() const {
    if (!IsValidRepository()) {
        return 0;
    }
    
    return m_repository->GetCount();
}

void ImageEditController::AddBoundingBox(const std::string& id, const BoundingBox& box) {
    ImageData* imageData = GetImageData(id);
    if (imageData) {
        imageData->AddBoundingBox(box);
    }
}

void ImageEditController::RemoveBoundingBox(const std::string& id, int index) {
    ImageData* imageData = GetImageData(id);
    if (imageData) {
        imageData->RemoveBoundingBox(index);
    }
}

void ImageEditController::RemoveBoundingBoxAt(const std::string& id, int x, int y) {
    ImageData* imageData = GetImageData(id);
    if (imageData) {
        imageData->RemoveBoundingBoxAt(x, y);
    }
}

void ImageEditController::ClearBoundingBoxes(const std::string& id) {
    ImageData* imageData = GetImageData(id);
    if (imageData) {
        imageData->ClearBoundingBoxes();
    }
}

std::vector<BoundingBox> ImageEditController::GetBoundingBoxes(const std::string& id) const {
    if (!IsValidRepository()) {
        return {};
    }
    
    ImageData* imageData = m_repository->GetImageData(id);
    if (!imageData) {
        return {};
    }
    
    return imageData->GetBoundingBoxes();
}

void ImageEditController::SetRepository(ImageDataRepository* repository) {
    m_repository = repository;
}

ImageDataRepository* ImageEditController::GetRepository() const {
    return m_repository;
}

bool ImageEditController::IsValidRepository() const {
    try {
        return m_repository != nullptr;
    } catch (...) {
        return false;
    }
}

std::string ImageEditController::GenerateImageName(const std::string& filepath) const {
    std::filesystem::path path(filepath);
    return path.filename().string();
}
