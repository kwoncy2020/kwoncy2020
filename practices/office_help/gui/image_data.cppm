module;
#include <opencv2/opencv.hpp>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

export module ImageData;
import Utils;


export class ImageData {
public:
    // Custom image format flags (library-agnostic)
    enum class ImageFormat {
        UNKNOWN = 0,
        GRAYSCALE = 1,
        RGB = 2,
        BGR = 3,
        RGBA = 4,
        BGRA = 5
    };
    
    enum class SourceType {
        STATIC_FILE,
        VIDEO_FRAME,
        WINDOW_CAPTURE,
        MEMORY_BUFFER,
        CAMERA_STREAM
    };


    struct SourceInfo{
        SourceType sourceType;
        std::string sourcePath;
        void* handle;
        int currentIndex;
        int totalElements;
        double fps;
        bool isDynamic;
    }
    

    // Constructors
    ImageData(const cv::Mat& image, ImageFormat format);
    ImageData(const cv::Mat& image, const std::string& filename, ImageFormat format);
    
    // Image operations
    void SetImage(const cv::Mat& image);
    cv::Mat GetImage() const { return m_image; }
    bool HasImage() const { return !m_image.empty(); }
    
    // Metadata operations
    void SetFilename(const std::string& filename) { m_filename = filename; }
    std::string GetFilename() const { return m_filename; }
    void SetCreationTime(const std::string& time) { m_creationTime = time; }
    std::string GetCreationTime() const { return m_creationTime; }
    void SetMetadata(const std::string& filename, const std::string& timestamp);
    
    // Image metadata operations
    int GetChannels() const { return m_channels; }
    int GetDepth() const { return m_depth; }
    size_t GetImageSize() const { return m_imageSize; }
    void UpdateImageMetadata();
    void SetImageFormat(ImageFormat format) { m_imageFormatEnum = format; }
    ImageFormat GetImageFormat() const { return m_imageFormatEnum; }
    std::string GetImageFormatString() const;
    // Bounding box operations
    void AddBoundingBox(const BoundingBox& box);
    void RemoveBoundingBox(int index);
    void RemoveBoundingBoxAt(int x, int y);
    void ModifyBoundingBox(int index, const BoundingBox& box);
    int FindBoundingBoxAt(int x, int y) const;
    std::vector<BoundingBox>& GetBoundingBoxes(const std::string& id) { return m_boundingBoxes; }
    BoundingBox GetBoundingBox(int index);
    void ClearBoundingBoxes();
    
    // Bounding box utilities (static)
    bool IsPointInBoundingBox(const BoundingBox& box, int x, int y);
    
        
    // Image properties
    int GetWidth() const { return m_image.cols; }
    int GetHeight() const { return m_image.rows; }
    cv::Size GetSize() const { return m_image.size(); }
    
private:
    cv::Mat m_image;           // Original image
    std::string m_filename;    // Original filename
    std::string m_creationTime; // File creation/modification time
    std::vector<BoundingBox> m_boundingBoxes;
    
    // Image metadata
    int m_channels;            // Number of channels (1=grayscale, 3=RGB, 4=RGBA)
    int m_depth;               // Bit depth (CV_8U, CV_16U, etc.)
    size_t m_imageSize;        // Total image size in bytes
    ImageFormat m_imageFormatEnum; // Custom image format (library-agnostic)
    std::string m_imageFormat; // Image format description
    
    // Helper methods
    std::string GetCurrentTimestamp() const;
    std::string GetDepthString(int depth) const;
};
