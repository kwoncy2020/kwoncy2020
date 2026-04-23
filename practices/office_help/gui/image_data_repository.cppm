module;
#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <mutex>

export module ImageDataRepository;

import ImageData;

export class ImageDataRepository {
public:
    // Singleton access
    static ImageDataRepository& GetInstance();
    
    // Instance management
    std::string CreateImageData(const cv::Mat& image, const std::string& name, ImageData::ImageFormat format);
    bool RemoveImageData(const std::string& id);
    ImageData* GetImageData(const std::string& id);
    std::vector<std::string> GetAllIds() const;
    
    // Metadata management
    void SetImageName(const std::string& id, const std::string& name);
    std::string GetImageName(const std::string& id) const;
    bool HasImageData(const std::string& id) const;
    
    // Statistics
    size_t GetCount() const;
    
private:
    ImageDataRepository() = default;
    ~ImageDataRepository() = default;
    ImageDataRepository(const ImageDataRepository&) = delete;
    ImageDataRepository& operator=(const ImageDataRepository&) = delete;
    
    std::unordered_map<std::string, std::unique_ptr<ImageData>> m_imageDataMap;
    std::unordered_map<std::string, std::string> m_nameMap;
    std::atomic<uint32_t> m_idCounter{0};
    mutable std::mutex m_mutex;
    
    // Helper methods
    std::string GenerateUniqueId();
};
