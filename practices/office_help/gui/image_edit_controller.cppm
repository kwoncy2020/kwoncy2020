module;
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>
#include <vector>
#include <algorithm>
#include <wx/bitmap.h>

export module ImageEditController;

import ImageDataRepository;
import ImageProcessor;
import ImageData;
import Utils;

export enum class ObserverSenderType{
    Controller,
    MainFrame,
    ScrollImageWindow,
    BoundingBoxControlPanel,
    ImageInfoPanel
};


export enum class DataChangeType {
        ImageLoaded,
        ImageRemoved,
        BoundingBoxAdded,
        BoundingBoxRemoved,
        BoundingBoxModified,
        BoundingBoxSelected,
        BoundingBoxUnSelected,
        BoundingBoxDisplayToggle,
        ImageMetadataChanged,
        ProcessingCompleted
    };

export class IDataObserver {
    public:
        virtual ~IDataObserver() = default;
        virtual void OnDataChanged(const std::string& imageId, DataChangeType changeType, ObserverSenderType observerSenderType, bool notifyToAll) = 0;
        virtual ObserverSenderType GetObserverSenderType() = 0;
    };


export class ImageEditController : public IDataObserver{
public:
    // Constructor - repository is required
    explicit ImageEditController(ImageDataRepository* repository);
    
    std::string OpenImageFile();
    // Image loading operations
    std::string LoadImageFromFile(const std::string& filepath, ImageData::ImageFormat format = ImageData::ImageFormat::BGR);
    // File name should be provided for tracking purposes. if the image not from the file then put any fileName you want.
    std::string LoadImageFromMat(const cv::Mat& image, const std::string& name = "", const std::string& imageFileName = "", ImageData::ImageFormat format = ImageData::ImageFormat::BGR);
    bool RemoveImage(const std::string& id);
    
    // Image access operations
    ImageData* GetImageData(const std::string& id);
    cv::Mat GetImage(const std::string& id);
    cv::Mat GetDisplayImage(const std::string& id, bool showBoundingBoxes = true, ImageData::ImageFormat targetFormat = ImageData::ImageFormat::RGB);  // get initial image.
    cv::Mat GetDisplayBoxedImage(const std::string& id, cv::Mat& convertedMat);  // get boxed image with RGB converted Mat and selectedIndex box. internally call GetDisplayImage.
    
    // Observer pattern for data changes - 확장성을 위한 중앙 관리
    void SetDataObserver(IDataObserver* observer);
    void AddDataObserver(IDataObserver* observer);
    void RemoveDataObserver(IDataObserver* observer);
    void NotifyDataChanged(const std::string& imageId, DataChangeType changeType);
    // gui가 컨트롤러에 요청을 할때 컨트롤러 역시 gui처럼 단일 루트로?
    void OnDataChanged(const std::string& imageId, DataChangeType changeType, ObserverSenderType observerSenderType, bool notifyToAll) override;
    ObserverSenderType GetObserverSenderType() override { return ObserverSenderType::Controller; };
    
    // Image format access
    std::string GetImageFormat(const std::string& id) const;
    
    // Image metadata operations
    std::string GetImageName(const std::string& id) const;
    void SetImageName(const std::string& id, const std::string& name);
    bool HasImage(const std::string& id) const;
    std::string GetLoadedImageFileName(const std::string& imageId);
    int GetImageWidth(const std::string& imageId);
    int GetImageHeight(const std::string& imageId);
    
    // Current image metadata getters
    std::string GetCurrentImageFileLoadPath() const;
    std::string GetCurrentImageId() const;
    std::string GetCurrentImageFileName() const;
    int GetCurrentImageWidth() const;
    int GetCurrentImageHeight() const;
    
    // Image listing operations
    std::vector<std::string> GetAllImageIds() const;
    size_t GetImageCount() const;
    
    // Bounding box operations
    void AddBoundingBox(const std::string& id, const BoundingBox& box);
    void RemoveBoundingBox(const std::string& id, int index);
    void RemoveBoundingBoxAt(const std::string& id, int x, int y);
    void ModifyBoundingBox(const std::string& id, int index, const BoundingBox& box);
    void ModifySelectedBoundingBox(const std::string& id, int selection, int dx, int dy, int dw, int dh, int stepSize);
    void ClearBoundingBoxes(const std::string& id);
    std::vector<BoundingBox>& GetBoundingBoxes(const std::string& id);
    BoundingBox GetBoundingBox(const std::string& id, int index) const;
    void SetBoundingBoxDisplayToggle(bool toggle);
    bool GetBoundingBoxDisplayToggle() const;

    // Selection operations
    void SetSelectedBoundingBoxIndex(const std::string& id, int index);
    void UnSetSelectedBoundingBoxIndex(const std::string& id);
    int GetSelectedBoundingBoxIndex(const std::string& id) const;
    
    void SetWXBitmapPixelAt(wxBitmap& bmp, int x, int y, unsigned char r, unsigned char g, unsigned char b);
    void UnSetWXBitmapPixelAt(wxBitmap& bmpOut, wxBitmap& bmpOrig, int x, int y);
    
    // Repository management
    void SetRepository(ImageDataRepository* repository);
    ImageDataRepository* GetRepository() const;
    
private:
    ImageDataRepository* m_repository;
    IDataObserver* m_observer;
    std::vector<IDataObserver*> m_dataObservers;
    
    // Image metadata
    std::string m_imageFileLoadPath;
    std::string m_imageOrigFileName;
    std::string m_imageId;
    int m_imageOrigWidth;
    int m_imageOrigHeight;
    std::map<std::string, int> m_selectedBoundingBoxIndices; // Store selected index per image

    //box status
    bool m_showBoundingBoxes;

    // Helper methods
    bool IsValidRepository() const;
    std::string GenerateImageName(const std::string& filepath) const;
    void updateInnerImageMetaInfo(std::string m_imageFileLoadPath, std::string m_imageId, std::string m_imageOrigFileName, int m_imageOrigWidth, int m_imageOrigHeight);
};
