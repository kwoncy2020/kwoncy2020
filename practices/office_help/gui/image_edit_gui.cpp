
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/filename.h>
#include <wx/spinctrl.h>
#include <opencv2/opencv.hpp>
import WindowFactory;
import ImageProcessor;
import ImageEditController;
import ImageDataRepository;
import ScrolledImageWindow;
import ControllerFactory;
import ImageData;
import ImageInfoPanel;
import BoundingBoxControlPanel;

class MainFrame : public wxFrame, public IDataObserver{
public:
    MainFrame() : wxFrame(NULL, wxID_ANY, "Office Help", wxDefaultPosition, wxSize(800, 600)),
                  m_imageOrigWidth(0), m_imageOrigHeight(0), m_controller(ControllerFactory::CreateController()) {
        m_controller->AddDataObserver(this);

        wxPrintf("MainFrame created with title: %s\n", GetTitle().ToStdString().c_str());
        
        // Create main sizer for the frame
        wxBoxSizer* mainSizerV = new wxBoxSizer(wxVERTICAL);
        
        wxNotebook* notebook = new wxNotebook(this, wxID_ANY);
        mainSizerV->Add(notebook, 1, wxEXPAND | wxALL, 5);


        // 탭 1: 시스템 설정
        wxPanel* settingPanel = new wxPanel(notebook);
        wxBoxSizer* settingSizer = new wxBoxSizer(wxVERTICAL);
        
        wxStaticText* settingText = new wxStaticText(settingPanel, wxID_ANY, wxString::FromUTF8("여기에 설정을 배치하세요."));
        settingSizer->Add(settingText, 1, wxALL, 20);
        settingPanel->SetSizer(settingSizer);

        // 탭 2: 이미지 분석 - 원래 레이아웃 복원
        wxPanel* imgAndCtlPanel = new wxPanel(notebook);
        wxBoxSizer* imgAndCtlPanelSizerH = new wxBoxSizer(wxHORIZONTAL);
        imgAndCtlPanel->SetSizer(imgAndCtlPanelSizerH);

        // wxPanel* scrollWindowOuterPanel = new wxPanel(imgAndCtlPanel);
        // wxBoxSizer* scrollWindowOuterSizerH = new wxBoxSizer(wxHORIZONTAL);
        // scrollWindowOuterPanel->SetSizer(scrollWindowOuterSizerH);

        // Left side: Image display
        // m_imageWindow = new ScrolledImageWindow(scrollWindowOuterPanel, wxID_ANY);
        m_imageWindow = new ScrolledImageWindow(imgAndCtlPanel, wxID_ANY);
        m_imageWindow->SetController(m_controller);
        m_imageWindow->SetMinSize(wxSize(400, -1));
        // wxBoxSizer* imageScrollWindowInnerSizerH = new wxBoxSizer(wxHORIZONTAL);
        // m_imageWindow->SetSizer(imageScrollWindowInnerSizerH);
        // scrollWindowOuterSizerH->Add(m_imageWindow, 1, wxEXPAND | wxALL, 5);
        // imgAndCtlPanelSizerH->Add(scrollWindowOuterSizerH, 1, wxEXPAND | wxALL, 5);
        imgAndCtlPanelSizerH->Add(m_imageWindow, 3, wxEXPAND | wxALL, 5);
        
        // Right side: Controls and information
        
        wxPanel* ctlAndBoxPanel = new wxPanel(imgAndCtlPanel);
        wxBoxSizer* ctlAndBoxPanelSizerV1 = new wxBoxSizer(wxVERTICAL);
        ctlAndBoxPanel->SetSizer(ctlAndBoxPanelSizerV1);
        
        // wxBoxSizer* rightCtlSizerV1 = new wxBoxSizer(wxVERTICAL);
        // Create and add ImageControlPanel
        m_imageInfoPanel = new ImageInfoPanel(ctlAndBoxPanel, m_controller);
        // rightCtlSizerV1->Add(m_imageInfoPanel, 1, wxEXPAND | wxALL, 5);
        // rightCtlSizerV1->Add(m_imageInfoPanel, 1, wxEXPAND | wxALL, 5);
        // m_imageInfoPanel->SetSizer(rightCtlSizerV1);
        // Create and add BoundingBoxControlPanel
        m_boundingBoxControlPanel = new BoundingBoxControlPanel(ctlAndBoxPanel, m_controller);
        // rightCtlSizerV1->Add(m_boundingBoxControlPanel, 1, wxEXPAND | wxALL, 5);
        // m_boundingBoxControlPanel->SetSizer(rightCtlSizerV1);
        ctlAndBoxPanelSizerV1->Add(m_imageInfoPanel, 1, wxEXPAND | wxALL, 5);
        ctlAndBoxPanelSizerV1->Add(m_boundingBoxControlPanel, 1, wxEXPAND | wxALL, 5);
        

        // ctlAndBoxPanelSizerV1->Add(m_boundingBoxControlPanel, 1, wxEXPAND | wxALL, 5);
        // ctlAndBoxPanelSizerV1->Add(m_imageInfoPanel, 1, wxEXPAND | wxALL, 5);
        // scrollWindowOuterPanel->SetSizer(scrollWindowOuterSizerH);
        // imgAndCtlPanelSizerH->Add(scrollWindowOuterPanel, 3, wxEXPAND | wxALL, 5);
        
        imgAndCtlPanelSizerH->Add(ctlAndBoxPanel, 1, wxEXPAND | wxALL, 5);
        notebook->AddPage(settingPanel, wxString::FromUTF8("시스템 설정"));
        notebook->AddPage(imgAndCtlPanel, wxString::FromUTF8("이미지 분석"));
        SetSizer(mainSizerV);
        // Ensure the layout is updated
        Layout();
    }

private:
    bool m_isOriginal = true;

    cv::Mat m_originalMat;
    cv::Mat m_croppedMat;
    cv::Mat m_currentMat;
    ScrolledImageWindow* m_imageWindow;
    ImageEditController* m_controller;
    
    // Image Original metadata storage
    wxString m_imageOrigFileName;
    int m_imageOrigWidth;
    int m_imageOrigHeight;
    
    // Image Cropped metadata storage
    int m_imageCroppedWidth;
    int m_imageCroppedHeight;
    
    // UI elements for image information and crop
    ImageInfoPanel* m_imageInfoPanel;
    BoundingBoxControlPanel* m_boundingBoxControlPanel;
    
    // should be modified.
    // void UpdateImageMetadata(bool isOriginal, cv::Mat& mat);
    void UpdateImageMetadata();
    
    
    // IMainFrameCallbacks should be removed
    void OnOpenViewerRequest() ;
    void OnCropButtonClick(wxCommandEvent& event);
    void OnCropRequest(int x, int y, int width, int height);
    
public:
    // IDataObserver implementation
    void OnDataChanged(const std::string& imageId, DataChangeType changeType, ObserverSenderType observerSenderType, bool notifyToAll) override;
    ObserverSenderType GetObserverSenderType() override {return ObserverSenderType::MainFrame;};
};

// TODO : Later
void MainFrame::OnCropRequest(int x, int y, int width, int height) {
    if (m_currentMat.empty()) {
        wxMessageBox(wxString::FromUTF8("먼저 이미지 파일을 열어주세요."), 
                    wxString::FromUTF8("알림"), wxOK | wxICON_INFORMATION);
        return;
    }
    
    // Validate crop region using ImageProcessor
    if (!ImageProcessor::ValidateCropRegion(m_currentMat, x, y, width, height)) {
        wxMessageBox(wxString::FromUTF8("Crop region is outside image boundaries."), 
                    wxString::FromUTF8("Error"), wxOK | wxICON_ERROR);
        return;
    }
    
    // Perform crop using ImageProcessor
    m_croppedMat = ImageProcessor::CropImage(m_currentMat, x, y, width, height);
    
    // Update UI with stored metadata
    // UpdateImageMetadata(false, m_croppedMat);
    // m_isOriginal = false;
    // UpdateImageMetadata()
    wxMessageBox(wxString::FromUTF8("이미지가 성공적으로 잘렸습니다."), 
                wxString::FromUTF8("성공"), wxOK | wxICON_INFORMATION);
}

// void MainFrame::UpdateImageMetadata(bool isOriginal, wxString imageOrigFileName, int imageOrigWidth, int imageOrigHeight) {
void MainFrame::UpdateImageMetadata() {
    m_imageOrigFileName = m_controller->GetCurrentImageFileName();
    if (m_isOriginal){
        m_imageOrigWidth = m_controller->GetCurrentImageWidth();
        m_imageOrigHeight = m_controller->GetCurrentImageHeight();
    }
    else {
        m_imageCroppedWidth = m_controller->GetCurrentImageWidth();
        m_imageCroppedHeight = m_controller->GetCurrentImageHeight();

    }
}

class MyApp : public wxApp {
    public:
        virtual bool OnInit() override {
            wxPrintf("Application starting...\n");
            MainFrame* frame = new MainFrame();
            wxPrintf("Frame created, showing window...\n");
            frame->Show(true);
            SetTopWindow(frame);
            wxPrintf("Application initialization complete.\n");
            return true;
        }
    };
    

void MainFrame::OnDataChanged(const std::string& imageId, DataChangeType changeType, ObserverSenderType observerSenderType, bool notifyToAll) {
    switch (changeType) {
        case DataChangeType::ImageLoaded:
        case DataChangeType::ImageRemoved:
        case DataChangeType::ImageMetadataChanged:
        case DataChangeType::BoundingBoxAdded:
        case DataChangeType::BoundingBoxRemoved:
        case DataChangeType::BoundingBoxModified:
        case DataChangeType::BoundingBoxSelected:
        case DataChangeType::BoundingBoxDisplayToggle:
            UpdateImageMetadata();
            break;
        case DataChangeType::ProcessingCompleted:
            // Handle processing completion
            break;
    }
}


wxIMPLEMENT_APP(MyApp);


