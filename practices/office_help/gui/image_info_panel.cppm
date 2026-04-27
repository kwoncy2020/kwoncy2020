module;

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/filename.h>
#include <opencv2/opencv.hpp>

export module ImageInfoPanel;

import ImageEditController;

export class ImageInfoPanel : public wxPanel, public IDataObserver {
public:

    // Constructor with dependency injection
    ImageInfoPanel(wxWindow* parent, 
                     ImageEditController* controller)
        : wxPanel(parent, wxID_ANY),
          m_controller(controller) {

        m_controller->AddDataObserver(this);
        InitializeUI();
        BindEvents();
    }


private:
    // Dependencies
    ImageEditController* m_controller;
    std::string m_currentImageId;  // Store current image ID
    
    // UI elements
    wxButton* m_openFileBtn;
    wxButton* m_openViewerBtn;
    wxStaticText* m_imageNameText;
    wxStaticText* m_imageSizeText;

    void InitializeUI() {
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

        // Button panel
        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
        
        // m_openFileBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("파일 열기"), wxDefaultPosition, wxSize(100,25));
        m_openFileBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("파일 열기"));
        // m_openViewerBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("확대 창 열기"), wxDefaultPosition, wxSize(100,25));
        m_openViewerBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("확대 창 열기"));
        
        buttonSizer->Add(m_openFileBtn, 1, wxALL, 5);
        buttonSizer->Add(m_openViewerBtn, 1, wxALL, 5);
        
        mainSizer->Add(buttonSizer, 1, wxALL, 5);

        // Image information panel
        wxStaticText* infoText = new wxStaticText(this, wxID_ANY,wxString::FromUTF8("이미지 정보"));
        mainSizer->Add(infoText, 1, wxTOP | wxLEFT | wxRIGHT, 5);
        
        // Image name display
        m_imageNameText = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("파일명: 없음"));
        mainSizer->Add(m_imageNameText, 1, wxEXPAND | wxALL, 5);
        
        // Image size display
        m_imageSizeText = new wxStaticText(this, wxID_ANY, wxString::FromUTF8("크기: 없음"));
        mainSizer->Add(m_imageSizeText, 1, wxEXPAND | wxALL, 5);

        SetSizer(mainSizer);
    }

    void BindEvents() {
        m_openFileBtn->Bind(wxEVT_BUTTON, &ImageInfoPanel::OnOpenFileButtonClick, this);
        m_openViewerBtn->Bind(wxEVT_BUTTON, &ImageInfoPanel::OnOpenViewerButtonClick, this);
    }

    void OnOpenFileButtonClick(wxCommandEvent& event) {
        // Direct controller call instead of callback
        std::string imageId = m_controller->OpenImageFile();
        if (!imageId.empty()) {
            // Image loaded successfully, UI will be updated via observer pattern
            // No need for callback since observer pattern handles updates
        }
    }

    void OnOpenViewerButtonClick(wxCommandEvent& event) {
        // Direct controller call instead of callback
        // TODO: Implement viewer functionality in controller
        // For now, keep empty as viewer is MainFrame specific
    }

    
    // Set current image ID for operations
    void SetCurrentImageId(const std::string& imageId) {
        m_currentImageId = imageId;
    }
    
    // Public interface for MainFrame to update display
    void UpdateImageInfo(const wxString& fileName, int width, int height) {
        if (m_imageNameText) {
            m_imageNameText->SetLabel(wxString::FromUTF8("파일명: ") + fileName);
        }
        if (m_imageSizeText) {
            wxString imageSize = wxString::Format(wxString::FromUTF8("크기: %dx%d"), width, height);
            m_imageSizeText->SetLabel(imageSize);
        }
    }

    void ClearImageInfo() {
        if (m_imageNameText) {
            m_imageNameText->SetLabel(wxString::FromUTF8("파일명: 없음"));
        }
        if (m_imageSizeText) {
            m_imageSizeText->SetLabel(wxString::FromUTF8("크기: 없음"));
        }
    }

    ObserverSenderType GetObserverSenderType() override {return ObserverSenderType::ImageInfoPanel;};
    
    // Observer pattern implementation
    void OnDataChanged(const std::string& imageId, DataChangeType changeType, ObserverSenderType senderType, bool notifyToAll = false) override {
        // Store current image ID when image is loaded
        if (changeType == DataChangeType::ImageLoaded) {
            m_currentImageId = imageId;
        }
    }

};

