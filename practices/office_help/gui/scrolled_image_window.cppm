module;
#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <wx/event.h>
#include <opencv2/opencv.hpp>
#include <vector>

export module ScrolledImageWindow;
import ImageProcessor;
import ImageData;
import ImageEditController;

export class ScrolledImageWindow : public wxScrolledWindow {
public:
    ScrolledImageWindow(wxWindow* parent, wxWindowID id = wxID_ANY, 
                       const wxPoint& pos = wxDefaultPosition, 
                       const wxSize& size = wxDefaultSize);
    
    // Controller injection
    void SetController(ImageEditController* controller);
    ImageEditController* GetController() const;
    
        
    void SetImage(const cv::Mat& image);
    void ClearImage();
    
    // Multi-instance ImageData management
    std::string CreateNewImageData(const cv::Mat& image, const std::string& name = "");
    void SetImageDataById(const std::string& id);
    std::string GetCurrentImageDataId() const;
    std::vector<std::string> GetAvailableImageDataIds() const;

private:
    // Image data
    cv::Mat m_originalImage;  // Original image without bounding boxes
    cv::Mat m_image;         // Display image (with bounding boxes applied)
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
    
    // Pre-calculated values for performance (updated only on size changes)
    double m_scrollRatioX;
    double m_scrollRatioY;
    float m_invUnitXInverse;
    float m_invUnitYInverse;
    
    // Separate scroll tracking for different modes
    wxPoint m_draggingScrollStart; // Scroll position when dragging started
    
    // Image data management
    std::string m_currentImageDataId;
    ImageEditController* m_controller;
    
    void OnSize(wxSizeEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseDoubleClick(wxMouseEvent& event);
    void OnMouseCaptureLost(wxMouseCaptureLostEvent& event);
    
    
    void updateValuesByResize();
    void UpdateDisplayImage();
    void DrawBoundingBoxes(wxPaintDC& dc);
    void ConvertAxisToImageAxis(int screenX, int screenY, int& imageX, int& imageY);
    wxPoint ConvertScreenToImage(const wxPoint& screenPos) const;
};
