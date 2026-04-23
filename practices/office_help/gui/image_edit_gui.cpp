
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/filename.h>
#include <opencv2/opencv.hpp>
import WindowFactory;
import ImageProcessor;
import ImageEditController;
import ImageDataRepository;
import ScrolledImageWindow;
import ControllerFactory;
import ImageData;

class MainFrame : public wxFrame {
public:
    MainFrame() : wxFrame(NULL, wxID_ANY, "Image Processing Tool", wxDefaultPosition, wxSize(800, 600)),
                  m_imageOrigWidth(0), m_imageOrigHeight(0), m_controller(ControllerFactory::CreateController()) {
        
        wxPrintf("MainFrame created with title: %s\n", GetTitle().ToStdString().c_str());
        
        // Create main sizer for the frame
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        
        wxNotebook* notebook = new wxNotebook(this, wxID_ANY);
        mainSizer->Add(notebook, 1, wxEXPAND | wxALL, 5);
        SetSizer(mainSizer);

        // 탭 1: 시스템 설정
        wxPanel* settingPanel = new wxPanel(notebook);
        wxBoxSizer* settingSizer = new wxBoxSizer(wxVERTICAL);
        
        wxStaticText* settingText = new wxStaticText(settingPanel, wxID_ANY, wxString::FromUTF8("여기에 설정을 배치하세요."));
        settingSizer->Add(settingText, 0, wxALL, 20);
        settingPanel->SetSizer(settingSizer);

        // 탭 2: 이미지 분석
        wxPanel* imgPanel = new wxPanel(notebook);
        wxBoxSizer* imgSizer = new wxBoxSizer(wxHORIZONTAL); // Horizontal main layout
        
        // Left side: Image display
        m_imageWindow = new ScrolledImageWindow(imgPanel, wxID_ANY);
        m_imageWindow->SetController(m_controller);
        imgSizer->Add(m_imageWindow, 1, wxEXPAND | wxALL, 10);
        
        // Right side: Controls and information
        wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
        
        // Button panel
        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
        
        wxButton* openFileBtn = new wxButton(imgPanel, wxID_ANY, wxString::FromUTF8("파일 열기"));
        openFileBtn->Bind(wxEVT_BUTTON, &MainFrame::OnOpenFileButtonClick, this);
        buttonSizer->Add(openFileBtn, 0, wxALL, 5);
        
        wxButton* openViewerBtn = new wxButton(imgPanel, wxID_ANY, wxString::FromUTF8("레이블 편집 창 열기"));
        openViewerBtn->Bind(wxEVT_BUTTON, &MainFrame::OnOpenViewerButtonClick, this);
        buttonSizer->Add(openViewerBtn, 0, wxALL, 5);
        
        rightSizer->Add(buttonSizer, 0, wxALL, 5);
        
        // Image information panel
        rightSizer->Add(new wxStaticText(imgPanel, wxID_ANY, wxString::FromUTF8("이미지 정보")), 0, wxTOP | wxLEFT | wxRIGHT, 5);
        
        // Image name display
        m_imageOrigNameText = new wxStaticText(imgPanel, wxID_ANY, wxString::FromUTF8("파일명: 없음"));
        rightSizer->Add(m_imageOrigNameText, 0, wxALL, 5);
        
        // Image size display
        m_imageOrigSizeText = new wxStaticText(imgPanel, wxID_ANY, wxString::FromUTF8("크기: 없음"));
        rightSizer->Add(m_imageOrigSizeText, 0, wxALL, 5);
        
        // Crop region input section
        rightSizer->Add(new wxStaticText(imgPanel, wxID_ANY, wxString::FromUTF8("자르기 영역")), 0, wxTOP | wxLEFT | wxRIGHT, 5);
        
        // Crop input fields
        wxBoxSizer* cropSizer = new wxBoxSizer(wxHORIZONTAL);
        cropSizer->Add(new wxStaticText(imgPanel, wxID_ANY, wxString::FromUTF8("X:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
        m_cropX = new wxTextCtrl(imgPanel, wxID_ANY, "0", wxDefaultPosition, wxSize(50, -1));
        cropSizer->Add(m_cropX, 0, wxALL, 2);
        
        cropSizer->Add(new wxStaticText(imgPanel, wxID_ANY, wxString::FromUTF8("Y:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
        m_cropY = new wxTextCtrl(imgPanel, wxID_ANY, "0", wxDefaultPosition, wxSize(50, -1));
        cropSizer->Add(m_cropY, 0, wxALL, 2);
        
        cropSizer->Add(new wxStaticText(imgPanel, wxID_ANY, wxString::FromUTF8("W:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
        m_cropWidth = new wxTextCtrl(imgPanel, wxID_ANY, "100", wxDefaultPosition, wxSize(50, -1));
        cropSizer->Add(m_cropWidth, 0, wxALL, 2);
        
        cropSizer->Add(new wxStaticText(imgPanel, wxID_ANY, wxString::FromUTF8("H:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
        m_cropHeight = new wxTextCtrl(imgPanel, wxID_ANY, "100", wxDefaultPosition, wxSize(50, -1));
        cropSizer->Add(m_cropHeight, 0, wxALL, 2);
        
        rightSizer->Add(cropSizer, 0, wxALL, 5);
        
        // Crop button
        wxButton* cropBtn = new wxButton(imgPanel, wxID_ANY, wxString::FromUTF8("자르기"));
        cropBtn->Bind(wxEVT_BUTTON, &MainFrame::OnCropButtonClick, this);
        rightSizer->Add(cropBtn, 0, wxALL, 5);
        
        imgSizer->Add(rightSizer, 0, wxEXPAND | wxALL, 10);
        
        imgPanel->SetSizer(imgSizer);

        notebook->AddPage(settingPanel, wxString::FromUTF8("시스템 설정"));
        notebook->AddPage(imgPanel, wxString::FromUTF8("이미지 분석"));
        
        // Ensure the layout is updated
        Layout();
    }

private:
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
    wxStaticText* m_imageOrigNameText;
    wxStaticText* m_imageOrigSizeText;
    wxStaticText* m_imageCroppedNameText;
    wxStaticText* m_imageCroppedSizeText;
    wxTextCtrl* m_cropX;
    wxTextCtrl* m_cropY;
    wxTextCtrl* m_cropWidth;
    wxTextCtrl* m_cropHeight;
    
    void OnOpenFileButtonClick(wxCommandEvent& event);
    void OnOpenViewerButtonClick(wxCommandEvent& event);
    void OnCropButtonClick(wxCommandEvent& event);
    void UpdateImageMetadata(bool isOriginal, cv::Mat& mat);
};

void MainFrame::OnOpenFileButtonClick(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, wxString::FromUTF8("이미지 파일 열기"), "", "",
                               "Image files (*.jpg;*.jpeg;*.png;*.bmp;*.tiff)|*.jpg;*.jpeg;*.png;*.bmp;*.tiff",
                               wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
    if (openFileDialog.ShowModal() == wxID_CANCEL) {
        return; // 사용자가 취소함
    }
    
    wxString filePath = openFileDialog.GetPath();
    
    // Use Controller to load image with default BGR format
    std::string filePathStr = filePath.ToStdString();
    std::string imageId = m_controller->LoadImageFromFile(filePathStr, ImageData::ImageFormat::BGR);
    
    if (imageId.empty()) {
        wxMessageBox(wxString::FromUTF8("이미지를 불러올 수 없습니다: ") + filePath, 
                    wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
        m_imageWindow->ClearImage();
        return;
    }
    
    m_currentMat = m_controller->GetImage(imageId);
    
    if (m_currentMat.empty()) {
        wxMessageBox(wxString::FromUTF8("이미지를 불러올 수 없습니다: ") + filePath, 
                    wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
        m_imageWindow->ClearImage();
        return;
    }
    
    // Display the loaded image in the scrolled window
    m_imageWindow->SetImage(m_currentMat);
    
    // Store image metadata
    m_imageOrigFileName = wxFileName(filePath).GetName();
    m_imageOrigWidth = m_currentMat.cols;
    m_imageOrigHeight = m_currentMat.rows;
    
    // Update UI with stored metadata
    UpdateImageMetadata(true, m_currentMat);
    
    wxMessageBox(wxString::FromUTF8("이미지가 성공적으로 로드되었습니다: ") + filePath, 
                wxString::FromUTF8("성공"), wxOK | wxICON_INFORMATION);
}

void MainFrame::OnOpenViewerButtonClick(wxCommandEvent& event) {
    if (m_currentMat.empty()) {
        wxMessageBox(wxString::FromUTF8("먼저 이미지 파일을 열어주세요."), 
                    wxString::FromUTF8("알림"), wxOK | wxICON_INFORMATION);
        return;
    }
    
    // 싱글톤 팩토리에 요청
    wxFrame* viewer = WindowManager::Get().CreateWindow(WindowType::TYPE_VIEWER, this, m_currentMat);
    if (viewer) {
        viewer->Show();
    }
}

void MainFrame::OnCropButtonClick(wxCommandEvent& event) {
    if (m_currentMat.empty()) {
        wxMessageBox(wxString::FromUTF8("먼저 이미지 파일을 열어주세요."), 
                    wxString::FromUTF8("알림"), wxOK | wxICON_INFORMATION);
        return;
    }
    
    // Get crop values from text controls
    long x, y, width, height;
    if (!m_cropX->GetValue().ToLong(&x) || !m_cropY->GetValue().ToLong(&y) || 
        !m_cropWidth->GetValue().ToLong(&width) || !m_cropHeight->GetValue().ToLong(&height)) {
        wxMessageBox(wxString::FromUTF8("잘못된 숫자 값입니다."), 
                    wxString::FromUTF8("오류"), wxOK | wxICON_ERROR);
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
    UpdateImageMetadata(false, m_croppedMat);
    
    wxMessageBox(wxString::FromUTF8("이미지가 성공적으로 잘렸습니다."), 
                wxString::FromUTF8("성공"), wxOK | wxICON_INFORMATION);
}

void MainFrame::UpdateImageMetadata(bool isOriginal, cv::Mat& mat) {
    if (isOriginal){
        m_imageOrigWidth = mat.cols;
        m_imageOrigHeight = mat.rows;
        // Update image name display
        m_imageOrigNameText->SetLabel(wxString::FromUTF8("파일명: ") + m_imageOrigFileName);
        
        // Update image size display
        wxString imageSize = wxString::Format(wxString::FromUTF8("크기: %dx%d"), m_imageOrigWidth, m_imageOrigHeight);
        m_imageOrigSizeText->SetLabel(imageSize);
    }
    else {
        m_imageCroppedWidth = mat.cols;
        m_imageCroppedHeight = mat.rows;
        // Update image name display
        m_imageCroppedNameText->SetLabel(wxString::FromUTF8("파일명: ") + m_imageOrigFileName + "_cropped");
        
        // Update image size display
        wxString imageSize = wxString::Format(wxString::FromUTF8("크기: %dx%d"), m_imageCroppedWidth, m_imageCroppedHeight);
        m_imageCroppedSizeText->SetLabel(imageSize);
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
    
wxIMPLEMENT_APP(MyApp);


