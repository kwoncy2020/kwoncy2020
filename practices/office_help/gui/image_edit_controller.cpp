module;

#include <opencv2/opencv.hpp>
#include <filesystem>
#include <wx/wx.h>
#include <wx/arrstr.h>
#include <map>
#include <wx/bitmap.h>
#include <wx/rawbmp.h>

module ImageEditController;

import ImageData;
import ImageDataRepository;
import ImageProcessor;

ImageEditController::ImageEditController(ImageDataRepository* repository)
    : m_repository(repository) {
    // Initialize observers vector
    m_dataObservers.clear();
}

std::string ImageEditController::OpenImageFile() {
    // Open file dialog
    wxFileDialog openFileDialog(nullptr, wxString::FromUTF8("Open Image File"), wxString::FromUTF8(""), wxString::FromUTF8(""), wxString::FromUTF8("Image files (*.jpg;*.png;*.bmp)|*.jpg;*.png;*.bmp"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return "";
    }
    
    std::string filePath = openFileDialog.GetPath().ToStdString();
    if (filePath.empty()) {
        return "";
    }
    
    // Load image using existing method, currently use opencv. so the format is BGR
    return LoadImageFromFile(filePath, ImageData::ImageFormat::BGR);
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
    cv::Mat image_ones = cv::Mat::ones(50,100,CV_8UC3);

    cv::Mat image = cv::imread(filepath, opencvFlags);
    if (image.empty()) {
        wxMessageBox(wxString::FromUTF8("이미지를 불러올 수 없습니다: ") + wxString(filepath.c_str(), wxConvUTF8),
                    wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
        return "";
    }
    

    // Generate name from filepath
    std::string name = GenerateImageName(filepath);
    
    // Create image data with format through repository. name should be the file name on this func.
    std::string imageId = m_repository->CreateImageData(image, name, name, format);
    
    if (!imageId.empty()) {
        // Notify observer that image was loaded
        updateInnerImageMetaInfo(filepath, imageId, name, image.cols, image.rows);
        NotifyDataChanged(imageId, DataChangeType::ImageLoaded);
        NotifyDataChanged(imageId, DataChangeType::ImageMetadataChanged);
        return imageId;
    }
    
    return "";
}

std::string ImageEditController::LoadImageFromMat(const cv::Mat& image, const std::string& name, const std::string& imageFileName, ImageData::ImageFormat format) {
    if (!IsValidRepository()) {
        return "";
    }
    
    if (image.empty() || imageFileName.empty()) {
        return "";
    }
    
    std::string imageName = name.empty() ? "unnamed_image" : name;
    return m_repository->CreateImageData(image, imageName, imageFileName, format);
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


cv::Mat ImageEditController::GetDisplayImage(const std::string& id, bool showBoundingBoxes, ImageData::ImageFormat targetFormat) {
    ImageData* imageData = GetImageData(id);
    if (!imageData) {
        return cv::Mat();
    }
    
    cv::Mat originalImage = imageData->GetImage();
    ImageData::ImageFormat currentFormatEnum = imageData->GetImageFormat();
    
    if (showBoundingBoxes) {
        std::vector<BoundingBox> boxes = imageData->GetBoundingBoxes(id);
        int selectedIndex = GetSelectedBoundingBoxIndex(id);
        // Draw bounding boxes and convert for display
        cv::Mat resultImage = ImageProcessor::DrawBoundingBoxesOnImage(originalImage, boxes, selectedIndex);
        return ImageProcessor::ConvertMatColorSpace(resultImage, currentFormatEnum, targetFormat);
    } else {
        // No bounding boxes, just convert original image for display
        return ImageProcessor::ConvertMatColorSpace(originalImage, currentFormatEnum, targetFormat);
    }
}

cv::Mat ImageEditController::GetDisplayBoxedImage(const std::string& id, cv::Mat& convertedMat){
    // Get Boxed Display Image without convert
    ImageData* imageData = GetImageData(id);
    if (!imageData) {
        return cv::Mat();
    }
    
    int selectedIndex = GetSelectedBoundingBoxIndex(id);
    // copied image will be returned
    return ImageProcessor::DrawBoundingBoxesOnImage(convertedMat, imageData->GetBoundingBoxes(id), selectedIndex);
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
        NotifyDataChanged(id, DataChangeType::BoundingBoxAdded);
    }
}

void ImageEditController::ClearBoundingBoxes(const std::string& id) {
    ImageData* imageData = GetImageData(id);
    if (imageData) {
        imageData->ClearBoundingBoxes();
        
        // Clear selected index when all boxes are cleared
        SetSelectedBoundingBoxIndex(id, -1);
        
        NotifyDataChanged(id, DataChangeType::BoundingBoxRemoved);
    }
}

void ImageEditController::ModifyBoundingBox(const std::string& id, int index, const BoundingBox& box) {
    ImageData* imageData = GetImageData(id);
    if (imageData) {
        imageData->ModifyBoundingBox(index, box);
        NotifyDataChanged(id, DataChangeType::BoundingBoxModified);
    }
}

void ImageEditController::ModifySelectedBoundingBox(const std::string& id, int selection, int dx, int dy, int dw, int dh, int stepSize) {
    ImageData* imageData = GetImageData(id);
    if (imageData) {
        auto boundingBoxes = imageData->GetBoundingBoxes(id);
        if (selection >= 0 && selection < boundingBoxes.size()) {
            const BoundingBox& currentBox = boundingBoxes[selection];
            
            // Apply modifications with step size
            int newX = std::max(0, currentBox.x + dx * stepSize);
            int newY = std::max(0, currentBox.y + dy * stepSize);
            int newW = std::max(1, currentBox.width + dw * stepSize);
            int newH = std::max(1, currentBox.height + dh * stepSize);
            
            // Create modified bounding box
            BoundingBox modifiedBox(newX, newY, newW, newH);
            
            // Update the bounding box
            imageData->ModifyBoundingBox(selection, modifiedBox);
            NotifyDataChanged(id, DataChangeType::BoundingBoxModified);
        }
    }
}

void ImageEditController::RemoveBoundingBox(const std::string& id, int index) {
    ImageData* imageData = GetImageData(id);
    if (imageData) {
        imageData->RemoveBoundingBox(index);
        
        // Clear selected index if the deleted box was selected
        int currentSelectedIndex = GetSelectedBoundingBoxIndex(id);
        if (currentSelectedIndex == index) {
            SetSelectedBoundingBoxIndex(id, currentSelectedIndex);
        } else if (currentSelectedIndex > index) {
            // Adjust selected index if a box before it was deleted
            SetSelectedBoundingBoxIndex(id, currentSelectedIndex - 1);
        }
        
        NotifyDataChanged(id, DataChangeType::BoundingBoxRemoved);
    }
}

void ImageEditController::RemoveBoundingBoxAt(const std::string& id, int x, int y) {
    ImageData* imageData = GetImageData(id);
    if (imageData) {
        // Get the index of the box at this position before removing
        std::vector<BoundingBox> boxes = imageData->GetBoundingBoxes(id);
        int removedIndex = -1;
        
        for (size_t i = boxes.size()-1; i >=0 ; --i) { // remove from the last one.
            const auto& box = boxes[i];
            if (x >= box.x && x < box.x + box.width &&
                y >= box.y && y < box.y + box.height) {
                removedIndex = static_cast<int>(i);
                break;
            }
        }
        
        imageData->RemoveBoundingBoxAt(x, y);
        
        // Clear selected index if the deleted box was selected
        if (removedIndex != -1) {
            int currentSelectedIndex = GetSelectedBoundingBoxIndex(id);
            if (currentSelectedIndex == removedIndex) {
                SetSelectedBoundingBoxIndex(id, -1);
            } else if (currentSelectedIndex > removedIndex) {
                // Adjust selected index if a box before it was deleted
                SetSelectedBoundingBoxIndex(id, currentSelectedIndex - 1);
            }
        }
        
        NotifyDataChanged(id, DataChangeType::BoundingBoxRemoved);
    }
}

std::vector<BoundingBox>& ImageEditController::GetBoundingBoxes(const std::string& id) {
    
    return m_repository->GetImageData(id)->GetBoundingBoxes(id);
}

BoundingBox ImageEditController::GetBoundingBox(const std::string& id, int index) const {
    if (!IsValidRepository()) {
        return BoundingBox();
    }
    
    ImageData* imageData = m_repository->GetImageData(id);
    if (!imageData) {
        return BoundingBox();
    }
    

    return imageData->GetBoundingBox(index);
}

bool ImageEditController::GetBoundingBoxDisplayToggle() const {
    return m_showBoundingBoxes;
}

void ImageEditController::SetBoundingBoxDisplayToggle(bool toggle) {
    if (!IsValidRepository()) {
        return;
    }
    m_showBoundingBoxes = toggle;
    NotifyDataChanged(m_imageId, DataChangeType::BoundingBoxDisplayToggle);
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

void ImageEditController::updateInnerImageMetaInfo(std::string imageFileLoadPath, std::string imageId, std::string imageOrigFileName, int imageOrigWidth, int imageOrigHeight) {
    m_imageFileLoadPath = imageFileLoadPath;
    m_imageId = imageId;
    m_imageOrigFileName = imageOrigFileName;
    m_imageOrigWidth = imageOrigWidth;
    m_imageOrigHeight = imageOrigHeight;
}

std::string ImageEditController::GetCurrentImageFileLoadPath() const {
    return m_imageFileLoadPath;
}

std::string ImageEditController::GetCurrentImageId() const {
    return m_imageId;
}

std::string ImageEditController::GetCurrentImageFileName() const {
    return m_imageOrigFileName;
}

int ImageEditController::GetCurrentImageWidth() const {
    return m_imageOrigWidth;
}

int ImageEditController::GetCurrentImageHeight() const {
    return m_imageOrigHeight;
}

void ImageEditController::SetDataObserver(IDataObserver* observer) {
    m_dataObservers.clear();
    if (observer) {
        m_dataObservers.push_back(observer);
    }
}

void ImageEditController::AddDataObserver(IDataObserver* observer) {
    if (observer && std::find(m_dataObservers.begin(), m_dataObservers.end(), observer) == m_dataObservers.end()) {
        m_dataObservers.push_back(observer);
    }
}

void ImageEditController::RemoveDataObserver(IDataObserver* observer) {
    auto it = std::find(m_dataObservers.begin(), m_dataObservers.end(), observer);
    if (it != m_dataObservers.end()) {
        m_dataObservers.erase(it);
    }
}

void ImageEditController::NotifyDataChanged(const std::string& imageId, DataChangeType changeType) {
    for (auto observer : m_dataObservers) {
        if (observer) {
            observer->OnDataChanged(imageId, changeType, ObserverSenderType::Controller, true);
        }
    }
}

// Not yet used
void  ImageEditController::OnDataChanged(const std::string& imageId, DataChangeType changeType, ObserverSenderType observerSenderType, bool notifyToAll) {
    // NotifyDataChanged(imageId, changeType);
}

void ImageEditController::SetSelectedBoundingBoxIndex(const std::string& id, int index) {
   if (index < 0 || m_repository->GetImageData(id)->GetBoundingBoxes(id).size() <= index) {
       return;
   }
    int oldIndex = GetSelectedBoundingBoxIndex(id);
    m_selectedBoundingBoxIndices[id] = index;
    
    // Notify observers about selection change
    if (oldIndex != index) {
        NotifyDataChanged(id, DataChangeType::BoundingBoxSelected);
    }
}

void ImageEditController::UnSetSelectedBoundingBoxIndex(const std::string& id) {
    m_selectedBoundingBoxIndices[id] = -1;
    NotifyDataChanged(id, DataChangeType::BoundingBoxUnSelected);
}

int ImageEditController::GetSelectedBoundingBoxIndex(const std::string& id) const {
    auto it = m_selectedBoundingBoxIndices.find(id);
    if (it != m_selectedBoundingBoxIndices.end()) {
        return it->second;
    }
    return -1; // No selection
}


void ImageEditController::SetWXBitmapPixelAt(wxBitmap& bmp, int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    // Set pixel at specified position
    ImageProcessor::SetWXBitmapPixelAt(bmp, x, y, r, g, b);
    // no require notify
}

void ImageEditController::UnSetWXBitmapPixelAt(wxBitmap& bmpOut, wxBitmap& bmpOrig, int x, int y) {
    // Unset pixel at specified position
    ImageProcessor::UnSetWXBitmapPixelAt(bmpOut, bmpOrig, x, y);
    // no require notify
}

