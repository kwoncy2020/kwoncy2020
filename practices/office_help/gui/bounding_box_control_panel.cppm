module;

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <opencv2/opencv.hpp>

export module BoundingBoxControlPanel;

import ImageEditController;
// import ScrolledImageWindow; 
import Utils; // for Bounding Box

export class BoundingBoxControlPanel : public wxPanel, public IDataObserver {
public:
    

    // Constructor with dependency injection
    BoundingBoxControlPanel(wxWindow* parent, 
                           ImageEditController* controller
                           )
        : wxPanel(parent, wxID_ANY),
          m_controller(controller)
         
        {
        m_controller->AddDataObserver(this);
        InitializeUI();
        BindEvents();
    }

    

private:
    // Dependencies
    ImageEditController* m_controller;
    
    std::string m_currentImageId;  // Store current image ID

    // UI elements
    wxCheckBox* m_showBoundingBoxCheckbox;
    wxListBox* m_boundingBoxListBox;
    wxTextCtrl* m_cropX;
    wxTextCtrl* m_cropY;
    wxTextCtrl* m_cropWidth;
    wxTextCtrl* m_cropHeight;
    wxSpinCtrl* m_stepSizeCtrl;
    wxButton* m_addBoundingBoxBtn;
    wxButton* m_cropBtn;
    wxButton* m_deleteBoundingBoxBtn;
    wxButton* m_xMinusBtn;
    wxButton* m_xPlusBtn;
    wxButton* m_yMinusBtn;
    wxButton* m_yPlusBtn;
    wxButton* m_wMinusBtn;
    wxButton* m_wPlusBtn;
    wxButton* m_hMinusBtn;
    wxButton* m_hPlusBtn;

    void InitializeUI() {
        wxBoxSizer* boundingBoxSizerV = new wxBoxSizer(wxVERTICAL);
        
        // 1. Display Control Section
        wxStaticBox* displayBox = new wxStaticBox(this, wxID_ANY, wxString::FromUTF8("표시 설정"));
        wxStaticBoxSizer* displaySizer = new wxStaticBoxSizer(displayBox, wxHORIZONTAL);
        m_showBoundingBoxCheckbox = new wxCheckBox(this, wxID_ANY, wxString::FromUTF8("바운딩박스 표시"));
        m_showBoundingBoxCheckbox->SetValue(true);
        displaySizer->Add(m_showBoundingBoxCheckbox, 1, wxALIGN_CENTER_VERTICAL | wxALL, 5);
        boundingBoxSizerV->Add(displaySizer, 2, wxEXPAND | wxALL, 5);
        
        // 2. List Management Section
        wxStaticBox* listBox = new wxStaticBox(this, wxID_ANY, wxString::FromUTF8("목록 관리"));
        listBox->SetMinSize(wxSize(200, 250)); // StaticBox 최소 크기 설정
        wxStaticBoxSizer* listSizer = new wxStaticBoxSizer(listBox, wxVERTICAL);
        
        m_boundingBoxListBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 120), 0, nullptr, wxLB_SINGLE);
        listSizer->Add(m_boundingBoxListBox, 1, wxEXPAND | wxALL, 5);
        
        m_deleteBoundingBoxBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("삭제"), wxDefaultPosition, wxSize(25,-1));
        listSizer->Add(m_deleteBoundingBoxBtn, 0, wxEXPAND | wxALL, 5);
        boundingBoxSizerV->Add(listSizer, 3, wxEXPAND | wxALL, 5);
        
        // 3. Box Control Section (merged add and modify)
        wxStaticBox* boxControlBox = new wxStaticBox(this, wxID_ANY, wxString::FromUTF8("박스 제어"));
        wxStaticBoxSizer* boxControlSizer = new wxStaticBoxSizer(boxControlBox, wxVERTICAL);
        
        // X,Y controls with + buttons row
        wxBoxSizer* xyRowSizer = new wxBoxSizer(wxHORIZONTAL);
        xyRowSizer->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8("X:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
        m_cropX = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(40,-1));
        xyRowSizer->Add(m_cropX, 1, wxEXPAND | wxALL, 2);
        
        xyRowSizer->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8("Y:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
        m_cropY = new wxTextCtrl(this, wxID_ANY, "0", wxDefaultPosition, wxSize(40,-1));
        xyRowSizer->Add(m_cropY, 1, wxEXPAND | wxALL, 2);
        
        // + buttons for X,Y
        m_xPlusBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("X+"), wxDefaultPosition, wxSize(25,-1));
        xyRowSizer->Add(m_xPlusBtn, 0, wxALL, 2);
        
        m_yPlusBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("Y+"), wxDefaultPosition, wxSize(25,-1));
        xyRowSizer->Add(m_yPlusBtn, 0, wxALL, 2);
        
        // + buttons for W,H (moved to X,Y row)
        m_wPlusBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("W+"), wxDefaultPosition, wxSize(25,-1));
        xyRowSizer->Add(m_wPlusBtn, 0, wxALL, 2);
        
        m_hPlusBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("H+"), wxDefaultPosition, wxSize(25,-1));
        xyRowSizer->Add(m_hPlusBtn, 0, wxALL, 2);
        
        boxControlSizer->Add(xyRowSizer, 0, wxEXPAND | wxALL, 5);
        
        // W,H controls and all - buttons row
        wxBoxSizer* whMinusRowSizer = new wxBoxSizer(wxHORIZONTAL);
        whMinusRowSizer->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8("W:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
        m_cropWidth = new wxTextCtrl(this, wxID_ANY, "100", wxDefaultPosition, wxSize(40,-1));
        whMinusRowSizer->Add(m_cropWidth, 1, wxEXPAND | wxALL, 2);
        
        whMinusRowSizer->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8("H:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
        m_cropHeight = new wxTextCtrl(this, wxID_ANY, "100", wxDefaultPosition, wxSize(40,-1));
        whMinusRowSizer->Add(m_cropHeight, 1, wxEXPAND | wxALL, 2);
        
        // - buttons for X,Y,W,H
        m_xMinusBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("X-"), wxDefaultPosition, wxSize(25,-1));
        whMinusRowSizer->Add(m_xMinusBtn, 0, wxALL, 2);
        
        m_yMinusBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("Y-"), wxDefaultPosition, wxSize(25,-1));
        whMinusRowSizer->Add(m_yMinusBtn, 0, wxALL, 2);
        
        m_wMinusBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("W-"), wxDefaultPosition, wxSize(25,-1));
        whMinusRowSizer->Add(m_wMinusBtn, 0, wxALL, 2);
        
        m_hMinusBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("H-"), wxDefaultPosition, wxSize(25,-1));
        whMinusRowSizer->Add(m_hMinusBtn, 0, wxALL, 2);
        
        boxControlSizer->Add(whMinusRowSizer, 0, wxEXPAND | wxALL, 5);
        
        // Step size and action buttons (third row)
        wxBoxSizer* bottomRowSizer = new wxBoxSizer(wxHORIZONTAL);
        bottomRowSizer->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8("변경값:")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 2);
        m_stepSizeCtrl = new wxSpinCtrl(this, wxID_ANY, "1");
        m_stepSizeCtrl->SetRange(1, 100);
        bottomRowSizer->Add(m_stepSizeCtrl, 0, wxALL, 2);
        
        m_addBoundingBoxBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("박스로 추가"), wxDefaultPosition, wxSize(60,-1));
        bottomRowSizer->Add(m_addBoundingBoxBtn, 1, wxALL, 2);
        
        m_cropBtn = new wxButton(this, wxID_ANY, wxString::FromUTF8("자르기"), wxDefaultPosition, wxSize(60,-1));
        bottomRowSizer->Add(m_cropBtn, 1, wxALL, 2);
        
        boxControlSizer->Add(bottomRowSizer, 0, wxEXPAND | wxALL, 5);
        boundingBoxSizerV->Add(boxControlSizer, 0, wxEXPAND | wxALL, 5);

        SetSizer(boundingBoxSizerV);
    }

    void BindEvents() {
        m_showBoundingBoxCheckbox->Bind(wxEVT_CHECKBOX, &BoundingBoxControlPanel::OnBoundingBoxDisplayToggle, this);
        m_boundingBoxListBox->Bind(wxEVT_LISTBOX, &BoundingBoxControlPanel::OnBoundingBoxSelected, this);
        m_addBoundingBoxBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnAddBoundingBoxButtonClick, this);
        m_cropBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnCropButtonClick, this);
        m_deleteBoundingBoxBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnDeleteBoundingBoxButtonClick, this);
        
        m_xMinusBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnXMinusButtonClick, this);
        m_xPlusBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnXPlusButtonClick, this);
        m_yMinusBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnYMinusButtonClick, this);
        m_yPlusBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnYPlusButtonClick, this);
        m_wMinusBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnWMinusButtonClick, this);
        m_wPlusBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnWPlusButtonClick, this);
        m_hMinusBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnHMinusButtonClick, this);
        m_hPlusBtn->Bind(wxEVT_BUTTON, &BoundingBoxControlPanel::OnHPlusButtonClick, this);
    }

    // Set current image ID for operations
    void SetCurrentImageId(const std::string& imageId) {
        m_currentImageId = imageId;
    }

    // Public interface for MainFrame to update display
    void UpdateBoundingBoxList(std::vector<BoundingBox>& boxes, bool setSelected = true) {
        if (!m_boundingBoxListBox) return;
        
        m_boundingBoxListBox->Clear();
        for (size_t i = 0; i < boxes.size(); ++i) {
            const auto& box = boxes[i];
            wxString item = wxString::Format("Box %zu: (%d, %d, %d, %d)", 
                                           i + 1, box.x, box.y, box.width, box.height);
            m_boundingBoxListBox->Append(item);
        }
        if (setSelected){
            int selectedIndex = m_controller->GetSelectedBoundingBoxIndex(m_currentImageId);
            if (selectedIndex >= 0 && m_boundingBoxListBox) {
                size_t count = m_boundingBoxListBox->GetCount();
                // Prevent if no item added in the box. count == 0 and index == 0. count should always bigger than index.
                if (count > selectedIndex) {
                    m_boundingBoxListBox->SetSelection(selectedIndex);
                }
            } else if (selectedIndex == -1 && m_boundingBoxListBox) {
                // Clear selection if no box is selected
                m_boundingBoxListBox->SetSelection(wxNOT_FOUND);
            }
        }
        
    }

    void ClearBoundingBoxList() {
        if (m_boundingBoxListBox) {
            m_boundingBoxListBox->Clear();
        }
    }

    void UpdateCropControls(int x, int y, int width, int height) {
        if (m_cropX) m_cropX->SetValue(wxString::Format("%d", x));
        if (m_cropY) m_cropY->SetValue(wxString::Format("%d", y));
        if (m_cropWidth) m_cropWidth->SetValue(wxString::Format("%d", width));
        if (m_cropHeight) m_cropHeight->SetValue(wxString::Format("%d", height));
    }

    int GetSelectedBoundingBoxIndex() const {
        if (m_boundingBoxListBox) {
            return m_boundingBoxListBox->GetSelection();
        }
        return wxNOT_FOUND;
    }

    bool GetCropValues(int& x, int& y, int& width, int& height) const {
        if (!m_cropX || !m_cropY || !m_cropWidth || !m_cropHeight) {
            return false;
        }
        
        long lx, ly, lw, lh;
        if (!m_cropX->GetValue().ToLong(&lx) || !m_cropY->GetValue().ToLong(&ly) || 
            !m_cropWidth->GetValue().ToLong(&lw) || !m_cropHeight->GetValue().ToLong(&lh)) {
            return false;
        }
        
        x = static_cast<int>(lx);
        y = static_cast<int>(ly);
        width = static_cast<int>(lw);
        height = static_cast<int>(lh);
        return true;
    }

    int GetStepSize() const {
        if (m_stepSizeCtrl) {
            return m_stepSizeCtrl->GetValue();
        }
        return 1;
    }

    
    void OnBoundingBoxDisplayToggle(wxCommandEvent& event) {
        bool showBoundingBoxes = m_showBoundingBoxCheckbox->GetValue();
        m_controller->SetBoundingBoxDisplayToggle(showBoundingBoxes);
    }

    void whenBoundingBoxSelected(){
        if (!m_controller || m_currentImageId.empty()) {
            return;
        }
        int selected = m_controller->GetSelectedBoundingBoxIndex(m_currentImageId);
        BoundingBox box = m_controller->GetBoundingBox(m_currentImageId, selected);
        UpdateCropControls(box.x, box.y, box.width, box.height);
    }

    void OnBoundingBoxSelected(wxCommandEvent& event) {
        int selection = m_boundingBoxListBox->GetSelection();
        if (m_controller && !m_currentImageId.empty()) {
            m_controller->SetSelectedBoundingBoxIndex(m_currentImageId, selection);
        }
    }

    void OnAddBoundingBoxButtonClick(wxCommandEvent& event) {
        int x, y, width, height;
        if (GetCropValues(x, y, width, height) && m_controller && !m_currentImageId.empty()) {
            BoundingBox box(x, y, width, height);
            m_controller->AddBoundingBox(m_currentImageId, box);
        }
    }

    void OnCropButtonClick(wxCommandEvent& event) {
        int x, y, width, height;
        // TODO : Crop Button method 구현
    }

    void OnDeleteBoundingBoxButtonClick(wxCommandEvent& event) {
        int selection = GetSelectedBoundingBoxIndex();
        if (selection != wxNOT_FOUND && m_controller && !m_currentImageId.empty()) {
            m_controller->RemoveBoundingBox(m_currentImageId, selection);
            // Unset the selected bounding box index
            m_controller->UnSetSelectedBoundingBoxIndex(m_currentImageId);
        }
    }

    void OnXMinusButtonClick(wxCommandEvent& event) {
        ModifySelectedBoundingBox(-1, 0, 0, 0);
    }

    void OnXPlusButtonClick(wxCommandEvent& event) {
        ModifySelectedBoundingBox(1, 0, 0, 0);
    }

    void OnYMinusButtonClick(wxCommandEvent& event) {
        ModifySelectedBoundingBox(0, -1, 0, 0);
    }

    void OnYPlusButtonClick(wxCommandEvent& event) {
        ModifySelectedBoundingBox(0, 1, 0, 0);
    }

    void OnWMinusButtonClick(wxCommandEvent& event) {
        ModifySelectedBoundingBox(0, 0, -1, 0);
    }

    void OnWPlusButtonClick(wxCommandEvent& event) {
        ModifySelectedBoundingBox(0, 0, 1, 0);
    }

    void OnHMinusButtonClick(wxCommandEvent& event) {
        ModifySelectedBoundingBox(0, 0, 0, -1);
    }

    void OnHPlusButtonClick(wxCommandEvent& event) {
        ModifySelectedBoundingBox(0, 0, 0, 1);
    }

    // ImageEditController::ModifySelectedBoundingBox 이미존재함
    void ModifySelectedBoundingBox(int dx, int dy, int dw, int dh) {
        int selection = m_controller->GetSelectedBoundingBoxIndex(m_currentImageId);
        if (selection != wxNOT_FOUND && m_controller && !m_currentImageId.empty()) {
            int stepSize = GetStepSize();
            m_controller->ModifySelectedBoundingBox(m_currentImageId, selection, dx, dy, dw, dh, stepSize);
        }
    }


    ObserverSenderType GetObserverSenderType() override {
        return ObserverSenderType::BoundingBoxControlPanel;
    }
    
    // Observer pattern implementation
    void OnDataChanged(const std::string& imageId, DataChangeType changeType, ObserverSenderType observerSenderType, bool notifyToAll) override {
        switch (changeType) {
            case DataChangeType::ImageLoaded:
                // Store current image ID when image is loaded
                m_currentImageId = imageId;
                break;
                
            case DataChangeType::BoundingBoxAdded: {
                std::vector<BoundingBox>& curBoxes = m_controller->GetBoundingBoxes(m_currentImageId);
                UpdateBoundingBoxList(curBoxes);
                Refresh();
                break;
            }
            case DataChangeType::BoundingBoxRemoved:
                UpdateBoundingBoxList(m_controller->GetBoundingBoxes(m_currentImageId));
                break;
            case DataChangeType::BoundingBoxModified:
                // Handle all bounding box events to maintain focus
                UpdateBoundingBoxList(m_controller->GetBoundingBoxes(m_currentImageId));
                break;
            case DataChangeType::BoundingBoxSelected:
                whenBoundingBoxSelected();
                UpdateBoundingBoxList(m_controller->GetBoundingBoxes(m_currentImageId));
                break;
            case DataChangeType::BoundingBoxDisplayToggle:
                // No need to update list as display toggle doesn't change box data
                break;
                
            case DataChangeType::ImageMetadataChanged:
                // Handle metadata changes if needed
                break;
                
            case DataChangeType::ProcessingCompleted:
                // Handle processing completion if needed
                break;
                
            case DataChangeType::ImageRemoved:
                // Clear current image ID when image is removed
                m_currentImageId.clear();
                if (m_boundingBoxListBox) {
                    m_boundingBoxListBox->Clear();
                }
                break;
        }
    }
};
