module;
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>

export module ImageEditController;

import ImageData;
import ImageDataRepository;
import ImageProcessor;

export class ImageEditController {
public:
    // Constructor
    ImageEditController();
    explicit ImageEditController(ImageDataRepository* repository);
    
    // Image loading operations
    std::string LoadImageFromFile(const std::string& filepath, ImageData::ImageFormat format = ImageData::ImageFormat::BGR);
    std::string LoadImageFromMat(const cv::Mat& image, const std::string& name = "", ImageData::ImageFormat format = ImageData::ImageFormat::BGR);
    bool RemoveImage(const std::string& id);
    
    // Image access operations
    ImageData* GetImageData(const std::string& id);
    cv::Mat GetImage(const std::string& id);
    cv::Mat GetDisplayImage(const std::string& id, bool showBoundingBoxes = true, ImageData::ImageFormat targetFormat = ImageData::ImageFormat::RGB);
    cv::Mat GetDisplayBoxedImage(const std::string& id, cv::Mat& convertedMat);
    
    // Image format access
    std::string GetImageFormat(const std::string& id) const;
    
    // Image metadata operations
    std::string GetImageName(const std::string& id) const;
    void SetImageName(const std::string& id, const std::string& name);
    bool HasImage(const std::string& id) const;
    
    // Image listing operations
    std::vector<std::string> GetAllImageIds() const;
    size_t GetImageCount() const;
    
    // Bounding box operations
    void AddBoundingBox(const std::string& id, const BoundingBox& box);
    void RemoveBoundingBox(const std::string& id, int index);
    void RemoveBoundingBoxAt(const std::string& id, int x, int y);
    void ClearBoundingBoxes(const std::string& id);
    std::vector<BoundingBox> GetBoundingBoxes(const std::string& id) const;
    
    // Repository management
    void SetRepository(ImageDataRepository* repository);
    ImageDataRepository* GetRepository() const;
    
private:
    ImageDataRepository* m_repository;
    
    // Helper methods
    bool IsValidRepository() const;
    std::string GenerateImageName(const std::string& filepath) const;
};
