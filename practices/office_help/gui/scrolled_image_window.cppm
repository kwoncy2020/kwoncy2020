module;
#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <wx/event.h>
#include <opencv2/opencv.hpp>
#include <vector>

export module ScrolledImageWindow;
// import ImageProcessor;
import Utils;
import ImageEditController;
import ImageData;

export class ScrolledImageWindow : public wxScrolledWindow, public IDataObserver {
public:
    ScrolledImageWindow(wxWindow* parent, wxWindowID id = wxID_ANY, 
                       const wxPoint& pos = wxDefaultPosition, 
                       const wxSize& size = wxDefaultSize
                       );
    // bug. is this method required?
    // virtual ~ScrolledImageWindow();
    
    // Controller injection
    void SetController(ImageEditController* controller);
    ImageEditController* GetController() const;
    
    void SetImage(const cv::Mat& image);
    void ClearImage();
    
    // Multi-instance ImageData management
    std::string CreateNewImageData(const cv::Mat& image, const std::string& name = "", const std::string& imageFileName = "", ImageData::ImageFormat format = ImageData::ImageFormat::BGR);
    void SetImageDataById(const std::string& id);
    std::string GetCurrentImageDataId() const;
    std::vector<std::string> GetAvailableImageDataIds() const;

    // IDataObserver implementation
    void OnDataChanged(const std::string& imageId, DataChangeType changeType, ObserverSenderType observerSenderType, bool notifyToAll) override;
    ObserverSenderType GetObserverSenderType() override {return ObserverSenderType::ScrollImageWindow;};

    // Bounding box display control
    void SetBoundingBoxDisplayMode(bool showBoundingBoxes);
    bool GetBoundingBoxDisplayMode() const;
    
    // Segmentation mode control
    cv::Mat InitializeSegmentationMask(const cv::Mat& mask);
    void SetSegmentationMode(bool enable);
    bool GetSegmentationMode() const;
    void ClearSegmentation();

    // Segmentation functionality
    void SetWXBitmapPixelAt(wxBitmap& bmp, int x, int y);
    void UnSetWXBitmapPixelAt(wxBitmap& bmpOut, wxBitmap& bmpOrig, int x, int y);
    void ApplyPixelOverlay(int x, int y);
    void PerformMaskSegmentation(int x, int y);
    void PerformPixelSegmentation(int imageX, int imageY, bool reaquestProcessor);
    cv::Mat ApplySegmentationOverlay(const cv::Mat& image, const cv::Mat& mask);

private:
    // Image data
    cv::Mat m_originalImage;      // Original image without bounding boxes
    cv::Mat m_imageDisplayMatRGB; // Display image in RGB format (with bounding boxes applied)
    int m_imageWidth;
    int m_imageHeight;  
    int m_clientWidth;
    int m_clientHeight; 
    float m_invUnitX;
    float m_invUnitY;
    bool m_isDragging;
    bool m_leftPressed;
    bool m_rightPressed;
    wxPoint m_dragStartPos;
    wxPoint m_drawingStartPos;
    wxPoint m_lastMousePos;
    
    // Bounding box functionality
    bool m_isDrawingBox;
    wxPoint m_currentBoxScreenStartPos;
    wxPoint m_currentBoxScreenPos;
    bool m_showBoundingBoxes;
    
    // Segmentation functionality
    unsigned char m_pixelWeightR=40;
    unsigned char m_pixelWeightG=127;
    unsigned char m_pixelWeightB=0;
    bool m_isSegmentationMode;
    wxBitmap m_segmentationImageRGB_Bitmap; // Copy of original for segmentation work
    cv::Mat m_segmentationMask;   // Binary mask for segmented areas
    bool m_showSegmentedImage;

    // Pre-calculated values for performance (updated only on size changes)
    double m_scrollRatioX;
    double m_scrollRatioY;
    float m_invUnitXInverse;
    float m_invUnitYInverse;
    
    // Separate scroll tracking for different modes
    wxPoint m_draggingScrollStart; // Scroll position when dragging started
    
    // Image data management
    std::string m_currentImageDataId;
    
    // Image caching for performance optimization
    wxBitmap m_cachedDisplayBitmap;
    bool m_isCacheValid;
    ImageEditController* m_controller;


    
    void OnSize(wxSizeEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseDoubleClick(wxMouseEvent& event);
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
    
    
    void updateValuesByResize();
    void DrawBoundingBoxes(wxPaintDC& dc);
    void ConvertAxisToImageAxis(int screenX, int screenY, int& imageX, int& imageY);
    wxPoint ConvertScreenToImage(const wxPoint& screenPos) const;


    
};
