module;
#include <sstream>
#include <mutex>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

module ImageDataRepository;


ImageDataRepository& ImageDataRepository::GetInstance() {
    static ImageDataRepository instance;
    return instance;
}

std::string ImageDataRepository::CreateImageData(const cv::Mat& image, const std::string& name, ImageData::ImageFormat format) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto id = GenerateUniqueId();
    m_imageDataMap[id] = std::make_unique<ImageData>(image, name.empty() ? id : name, format);
    m_nameMap[id] = name.empty() ? id : name;
    
    return id;
}

bool ImageDataRepository::RemoveImageData(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_imageDataMap.find(id);
    if (it != m_imageDataMap.end()) {
        m_imageDataMap.erase(it);
        m_nameMap.erase(id);
        return true;
    }
    return false;
}

ImageData* ImageDataRepository::GetImageData(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_imageDataMap.find(id);
    if (it != m_imageDataMap.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<std::string> ImageDataRepository::GetAllIds() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::vector<std::string> ids;
    ids.reserve(m_imageDataMap.size());
    
    for (const auto& pair : m_imageDataMap) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

void ImageDataRepository::SetImageName(const std::string& id, const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_imageDataMap.find(id) != m_imageDataMap.end()) {
        m_nameMap[id] = name;
    }
}

std::string ImageDataRepository::GetImageName(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_nameMap.find(id);
    if (it != m_nameMap.end()) {
        return it->second;
    }
    return id; // Fallback to ID if name not found
}

bool ImageDataRepository::HasImageData(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_imageDataMap.find(id) != m_imageDataMap.end();
}

size_t ImageDataRepository::GetCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_imageDataMap.size();
}

std::string ImageDataRepository::GenerateUniqueId() {
    // Generate unique ID using atomic counter
    auto id = m_idCounter.fetch_add(1);
    std::stringstream ss;
    ss << "img_" << id;
    return ss.str();
}
