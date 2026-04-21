
#include <wx/wx.h>
#include <wx/notebook.h>
#include <opencv2/opencv.hpp> 


import WindowFactory;

class MainFrame : public wxFrame {
public:
    MainFrame() : wxFrame(NULL, wxID_ANY, "Image Processing Tool", wxDefaultPosition, wxSize(800, 600)) {
        wxNotebook* notebook = new wxNotebook(this, wxID_ANY);

        // 탭 1: 이미지 관리
        wxPanel* imgPanel = new wxPanel(notebook);
        wxButton* openViewerBtn = new wxButton(imgPanel, wxID_ANY, "새 이미지 뷰어 열기", wxPoint(20, 20));
        openViewerBtn->Bind(wxEVT_BUTTON, &MainFrame::OnOpenViewerButtonClick, this);

        // 탭 2: 설정
        wxPanel* settingPanel = new wxPanel(notebook);
        new wxStaticText(settingPanel, wxID_ANY, "여기에 설정을 배치하세요.", wxPoint(20, 20));

        notebook->AddPage(imgPanel, "이미지 분석");
        notebook->AddPage(settingPanel, "시스템 설정");
    }

private:
    cv::Mat m_currentMat;
    void OnOpenViewerButtonClick(wxCommandEvent& event);
};

void MainFrame::OnOpenViewerButtonClick(wxCommandEvent& event) {
    // 싱글톤 팩토리에 요청
    wxFrame* viewer = WindowManager::Get().CreateWindow(WindowType::TYPE_VIEWER, this, m_currentMat);
    if (viewer) {
        viewer->Show();
    }
}

class MyApp : public wxApp {
    public:
        virtual bool OnInit() override {
            MainFrame* frame = new MainFrame();
            frame->Show(true);
            SetTopWindow(frame);
            return true;
        }
    };
    
wxIMPLEMENT_APP(MyApp);


