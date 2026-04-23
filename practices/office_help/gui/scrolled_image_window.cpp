module;
#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <opencv2/opencv.hpp>

module ScrolledImageWindow;
import ImageData;
import ImageProcessor;
import ImageEditController;
import Utils;

ScrolledImageWindow::ScrolledImageWindow(wxWindow* parent, wxWindowID id, 
                                       const wxPoint& pos, 
                                       const wxSize& size)
    : wxScrolledWindow(parent, id, pos, size), 
      m_isDragging(false), m_leftPressed(false), m_rightPressed(false),
      m_isDrawingBox(false), m_currentBoxScreenPos(0, 0),
      m_scrollRatioX(1.0), m_scrollRatioY(1.0),
      m_controller(nullptr) {
    
    SetScrollRate(10, 10); // Scroll rate in pixels
    SetBackgroundColour(wxColour(240, 240, 240));
    
    // Enable double buffering for smoother rendering
    SetBackgroundStyle(wxBG_STYLE_PAINT);  // Enable double buffering
    SetDoubleBuffered(true);               // Explicit double buffering
    
    Bind(wxEVT_PAINT, &ScrolledImageWindow::OnPaint, this);
    Bind(wxEVT_SIZE, &ScrolledImageWindow::OnSize, this);
    
    // Mouse events for dragging and bounding box
    Bind(wxEVT_LEFT_DOWN, &ScrolledImageWindow::OnMouseDown, this);
    Bind(wxEVT_RIGHT_DOWN, &ScrolledImageWindow::OnMouseDown, this);
    Bind(wxEVT_LEFT_UP, &ScrolledImageWindow::OnMouseUp, this);
    Bind(wxEVT_RIGHT_UP, &ScrolledImageWindow::OnMouseUp, this);
    Bind(wxEVT_MOTION, &ScrolledImageWindow::OnMouseMove, this);
    Bind(wxEVT_RIGHT_DCLICK, &ScrolledImageWindow::OnMouseDoubleClick, this);
    Bind(wxEVT_MOUSE_CAPTURE_LOST, &ScrolledImageWindow::OnMouseCaptureLost, this);
    
    // Set cursor for better user experience
    SetCursor(wxCursor(wxCURSOR_HAND));
}

void ScrolledImageWindow::SetImage(const cv::Mat& image) {
    if (image.empty()) {
        m_originalImage = cv::Mat();
        m_image = cv::Mat();
        SetVirtualSize(100, 100); // Default size when no image
        m_imageWidth = 0;
        m_imageHeight = 0;
        Refresh();
        return;
    }
    
    // Store original image
    m_originalImage = image.clone();
    
    // Create new ImageData through controller
    if (!m_controller) {
        return; // Controller must be set
    }
    
    m_currentImageDataId = m_controller->LoadImageFromMat(image);
    
    // Get display image from controller
    m_image = m_controller->GetDisplayImage(m_currentImageDataId);
    
    // Set virtual size to image dimensions
    SetVirtualSize(m_image.cols, m_image.rows);
    m_imageWidth = m_image.cols;
    m_imageHeight = m_image.rows;
    
    updateValuesByResize();
    Refresh();
}

void ScrolledImageWindow::ClearImage() {
    m_originalImage = cv::Mat();
    m_image = cv::Mat();
    m_imageWidth = 0;
    m_imageHeight = 0;
    SetVirtualSize(100, 100);
    Refresh();
}

void ScrolledImageWindow::UpdateDisplayImage() {
    if (m_originalImage.empty()) {
        m_image = cv::Mat();
        return;
    }
    
    // Get display image from controller (which includes bounding boxes)
    if (!m_controller) {
        m_image = m_originalImage.clone();
        return;
    }
    
    if (m_controller && !m_currentImageDataId.empty()) {
        m_image = m_controller->GetDisplayImage(m_currentImageDataId);
    } else {
        m_image = m_originalImage.clone();
    }
}

void ScrolledImageWindow::OnSize(wxSizeEvent& event) {
    updateValuesByResize();
    
    Refresh();
    event.Skip();
}

void ScrolledImageWindow::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    PrepareDC(dc);   
    
    if (m_image.empty()) {
        // Draw placeholder text when no image is loaded
        dc.SetBrush(GetBackgroundColour());
        dc.Clear();
        dc.SetTextForeground(wxColour(128, 128, 128));
        dc.DrawText(wxString::FromUTF8("No image loaded"), wxPoint(10, 10));
        return;
    }
    
    // Convert OpenCV Mat to wxBitmap for display
    cv::Mat converted;
    if (m_image.channels() == 3) {
        cv::cvtColor(m_image, converted, cv::COLOR_BGR2RGB);
    } else if (m_image.channels() == 4) {
        cv::cvtColor(m_image, converted, cv::COLOR_BGRA2RGB);
    } else if (m_image.channels() == 1) {
        cv::cvtColor(m_image, converted, cv::COLOR_GRAY2RGB);
    } else {
        converted = m_image.clone();
    }
    
    wxImage wxImg(converted.cols, converted.rows, converted.data, true);
    wxBitmap bitmap(wxImg);
    
    // Draw the image at the origin of the scrolled window
    dc.DrawBitmap(bitmap, 0, 0, false);
    
    // Draw bounding boxes if any exist
    DrawBoundingBoxes(dc);
}

void ScrolledImageWindow::OnMouseDown(wxMouseEvent& event) {
    if (m_image.empty()) return;
    if (event.LeftDown()) m_leftPressed = true;
    if (event.RightDown()) m_rightPressed = true;
    
    if (m_leftPressed) {
        m_isDrawingBox = true;
    }
    if (m_leftPressed && m_rightPressed) {
        m_isDrawingBox = false;
        // // Reset current box and drawing
        // m_currentBoxScreenPos = wxPoint(0, 0);
        m_isDragging = true;
    }

    if (m_isDrawingBox) {
        wxPoint pos = event.GetPosition();
        // save box start position
        // Convert screen coordinates to image coordinates
        m_currentBoxScreenStartPos = pos;
        // ConvertAxisToImageAxis(pos.x, pos.y, m_currentBoxScreenStartPos.x, m_currentBoxScreenStartPos.y);
    }

    if (m_isDragging){
        // GetViewStart(&m_draggingScrollStart.x, &m_draggingScrollStart.y);
        m_dragStartPos = event.GetPosition();
        m_lastMousePos = m_dragStartPos;
    }

    if (m_isDrawingBox || m_isDragging) {
        if(m_isDrawingBox) {
            SetCursor(wxCursor(wxCURSOR_CROSS));
        }else if (m_isDragging) {
            SetCursor(wxCursor(wxCURSOR_SIZING));
        }
        if (!HasCapture()) {
            CaptureMouse();
        }
    }

    event.Skip();
}


void ScrolledImageWindow::OnMouseUp(wxMouseEvent& event) {
    if (event.LeftUp()) m_leftPressed = false;
    if (event.RightUp()) m_rightPressed = false;
    
    if (m_isDrawingBox && event.LeftUp()) {
        // Finalize bounding box drawing
        m_isDrawingBox = false;
        wxPoint curPos = event.GetPosition();
        auto [x,y,w,h] = Utils::getBoxValuesFromPoints(m_currentBoxScreenStartPos.x, m_currentBoxScreenStartPos.y, curPos.x, curPos.y, "xywh");
        
        // Add the bounding box if it has valid dimensions
        if (w > 5 && h > 5) {
            int startImageX, startImageY;
            // int startImageX, startImageY, endImageX, endImageY;
            ConvertAxisToImageAxis(x, y, startImageX, startImageY);
            // ConvertAxisToImageAxis(endX, endY, endImageX, endImageY);
            BoundingBox box(startImageX, startImageY, w, h);
            if (m_controller) {

                m_controller->AddBoundingBox(m_currentImageDataId, box);
            }
            // Update display image with new bounding box
            UpdateDisplayImage();
            if (HasCapture()) {
            ReleaseMouse();
            }
            SetCursor(wxCursor(wxCURSOR_HAND));
            return;
        }
    }
    
    // Stop scrolling if both buttons are released
    if ((m_isDragging && !m_leftPressed) || (m_isDragging && !m_rightPressed)) {
        m_isDragging = false;
        
    }
    // Additional safety: release capture if no operations are active
    if (!m_isDrawingBox && !m_isDragging) {
        if (HasCapture()) {
            ReleaseMouse();
        }
        SetCursor(wxCursor(wxCURSOR_HAND));
    }

    event.Skip();
}

void ScrolledImageWindow::OnMouseMove(wxMouseEvent& event) {
    if (!m_image.empty()) {
            // Trigger refresh to show the box being drawn
            Refresh();
        }
        
    // Handle scrolling (left + right click)
    if (m_isDragging) {
        wxPoint currentPos = event.GetPosition();
        int deltaX = m_lastMousePos.x - currentPos.x;
        int deltaY = m_lastMousePos.y - currentPos.y;
        
        // Add movement threshold to prevent stuttering when button is held
        const int MOVEMENT_THRESHOLD = 3; // Increased threshold for smoother experience
        if (abs(deltaX) < MOVEMENT_THRESHOLD && abs(deltaY) < MOVEMENT_THRESHOLD) {
            event.Skip(); // Skip processing if no significant movement
            return;
        }
        
        // Get current scroll position
        int scrollX, scrollY;
        GetViewStart(&scrollX, &scrollY);
        
        // Apply proportional scrolling with optimized damping
        // Use pre-computed values to avoid division
        const float dampingFactor = 0.12f; // Further reduced for smoother movement
        
        // Calculate adjusted deltas using pre-computed inverse values
        // This avoids division operations in the hot path
        int adjustedDeltaX = (int)(deltaX * m_invUnitXInverse * dampingFactor);
        int adjustedDeltaY = (int)(deltaY * m_invUnitYInverse * dampingFactor);
        
        // Apply scroll with boundary checking
        Scroll(scrollX + adjustedDeltaX, scrollY + adjustedDeltaY);

        m_lastMousePos = currentPos;
    }
    else if(m_isDrawingBox) {
        // modify  m_currentBoxScreenPos values based on mouse position
        wxPoint pos = event.GetPosition();
        m_currentBoxScreenPos = pos;
        Refresh();
    }
    event.Skip();
}


void ScrolledImageWindow::OnMouseDoubleClick(wxMouseEvent& event) {
    if (event.RightDClick() && !m_image.empty()) {
        // Get mouse position in image coordinates
        wxPoint pos = event.GetPosition();
        int imageX, imageY;
        ConvertAxisToImageAxis(pos.x, pos.y, imageX, imageY);
        
        // Remove bounding box at this position
        if (m_controller && !m_currentImageDataId.empty()) {
            m_controller->RemoveBoundingBoxAt(m_currentImageDataId, imageX, imageY);
        }
        
        // Update display image with modified bounding boxes
        UpdateDisplayImage();
        Refresh();
    }
    event.Skip();
}

void ScrolledImageWindow::DrawBoundingBoxes(wxPaintDC& dc) {
    // Draw current box being drawn (dynamic bounding box)
    if (m_isDrawingBox) {
        // OnPaint 함수 내에서 PrepareDC(dc)를 호출한 뒤 화면에 바운딩 박스를 그리는 것이기 때문에
        // 클라이언트 좌표를 그대로 사용할 수 없다. 마치 이미지 좌표계를 사용하듯이 해야함.   
        // 좌클릭+우클릭으로 스크롤을 이동하는 함수가  PrepareDC(dc)아래에서 동작하기에 비활성 할 수 없음.
        
        auto [x,y,w,h] = Utils::getBoxValuesFromPoints(m_currentBoxScreenStartPos.x, m_currentBoxScreenStartPos.y, m_currentBoxScreenPos.x, m_currentBoxScreenPos.y, "xywh");
        int imageX, imageY;
        ConvertAxisToImageAxis(x, y, imageX, imageY);
        
        // Only draw if box has valid dimensions
        if (w > 0 && h > 0) {
            // Set pen for green outline
            dc.SetPen(wxPen(wxColour(0, 255, 0), 2)); // Green pen for current box
            
            // Set transparent brush for drawing (no fill)
            dc.SetBrush(wxBrush(wxColour(0, 255, 0), wxBRUSHSTYLE_TRANSPARENT)); // Transparent brush
            
            // wxPoint mousePos = event.GetPosition();
            // int imageX, imageY;
            // CalcScrolledPosition(mousePos.x, mousePos.y, &imageX, &imageY);
            dc.DrawRectangle(imageX,imageY,w,h);
            
        } 
    }
    
    // Note: Static bounding boxes are now drawn on the image itself
    // by ImageProcessor::DrawBoundingBoxesOnImage() in SetImage()
}

void ScrolledImageWindow::updateValuesByResize() {

    GetClientSize(&m_clientWidth, &m_clientHeight);
    
    // Keep fixed scroll rate for smooth scrolling
    SetScrollRate(10, 10);
    
    // Calculate screen-to-image ratios for proper coordinate conversion
    if (m_imageWidth > 0 && m_clientWidth > 0) {
        m_invUnitX = (float)m_clientWidth / (float)m_imageWidth;
    } else {
        m_invUnitX = 1.0f;
    }
    
    if (m_imageHeight > 0 && m_clientHeight > 0) {
        m_invUnitY = (float)m_clientHeight / (float)m_imageHeight;
    } else {
        m_invUnitY = 1.0f;
    }

    // Pre-compute values for OnMouseMove optimization
    m_scrollRatioX = 1.0;
    m_scrollRatioY = 1.0;
    
    if (m_imageWidth > 0 && m_clientWidth > 0) {
        m_scrollRatioX = (double)m_imageWidth / m_clientWidth;
    }
    if (m_imageHeight > 0 && m_clientHeight > 0) {
        m_scrollRatioY = (double)m_imageHeight / m_clientHeight;
    }
    
    // Pre-compute inverse values to avoid division in OnMouseMove
    if (m_invUnitX > 0.0f) {
        m_invUnitXInverse = 1.0f / m_invUnitX;
    } else {
        m_invUnitXInverse = 1.0f;
    }
    
    if (m_invUnitY > 0.0f) {
        m_invUnitYInverse = 1.0f / m_invUnitY;
    } else {
        m_invUnitYInverse = 1.0f;
    }
}


void ScrolledImageWindow::ConvertAxisToImageAxis(int ClientX, int ClientY, int& imageX, int& imageY) {
    // CalcUnscrolledPosition(screenX, screenY, &imageX, &imageY);  // DoprepareDC(dc) 함수를 호출해야 제대로작동함.
    
    // 1. 마우스의 클라이언트(화면) 좌표
    // wxPoint clientPos = event.GetPosition();

    // 2. 현재 스크롤된 픽셀 오프셋 직접 가져오기
    int xX, xY;
    GetViewStart(&xX, &xY); // 스크롤 유닛 단위 위치
    int xUnit, yUnit;
    GetScrollPixelsPerUnit(&xUnit, &yUnit); // 유닛당 픽셀 수

    // 3. 실제 이미지상의 좌표 계산 (CalcUnscrolledPosition의 수동 구현 버전)
    imageX = ClientX + (xX * xUnit);
    imageY =  ClientY + (xY * yUnit);
}

// wxPoint ScrolledImageWindow::ConvertScreenToImage(const wxPoint& screenPos) const {
//     if (m_image.empty()) return wxPoint(0, 0);
    
//     // Convert screen coordinates to image coordinates considering scroll position
//     int scrollX, scrollY;
//     GetViewStart(&scrollX, &scrollY);
    
//     int imageX = (screenPos.x + scrollX) * m_invUnitXInverse;
//     int imageY = (screenPos.y + scrollY) * m_invUnitYInverse;
    
//     return wxPoint(imageX, imageY);
// }

// Controller injection methods
void ScrolledImageWindow::SetController(ImageEditController* controller) {
    m_controller = controller;
}

ImageEditController* ScrolledImageWindow::GetController() const {
    return m_controller;
}

// Multi-instance ImageData management methods
std::string ScrolledImageWindow::CreateNewImageData(const cv::Mat& image, const std::string& name) {
    if (!m_controller) {
        return ""; // Controller must be set
    }
    
    std::string id = m_controller->LoadImageFromMat(image, name);
    SetImageDataById(id);
    return id;
}

void ScrolledImageWindow::SetImageDataById(const std::string& id) {
    if (!m_controller) {
        return; // Controller must be set
    }
    
    ImageData* imageData = m_controller->GetImageData(id);
    
    if (imageData && imageData->HasImage()) {
        m_currentImageDataId = id;
        m_originalImage = imageData->GetImage();
        m_image = m_controller->GetDisplayImage(m_currentImageDataId);
        
        // Set virtual size to image dimensions
        SetVirtualSize(m_image.cols, m_image.rows);
        m_imageWidth = m_image.cols;
        m_imageHeight = m_image.rows;
        
        updateValuesByResize();
        Refresh();
    }
}

std::string ScrolledImageWindow::GetCurrentImageDataId() const {
    return m_currentImageDataId;
}

std::vector<std::string> ScrolledImageWindow::GetAvailableImageDataIds() const {
    if (!m_controller) {
        return {}; // Controller must be set
    }
    
    return m_controller->GetAllImageIds();
}

void ScrolledImageWindow::OnMouseCaptureLost(wxMouseCaptureLostEvent& event) {
    // Reset all drawing and dragging states when mouse capture is lost
    m_isDrawingBox = false;
    m_isDragging = false;
    m_leftPressed = false;
    m_rightPressed = false;
    
    // Reset cursor to default
    SetCursor(wxCursor(wxCURSOR_HAND));
    
    // Force refresh to clear any partial drawing
    Refresh();
    
    // Skip the event to allow default processing
    event.Skip();
}
