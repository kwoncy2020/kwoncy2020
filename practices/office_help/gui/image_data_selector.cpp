module
#include <sstream>


module ImageDataSelector;


// Event implementation
wxDEFINE_EVENT(ImageDataSelectionChangedEvent, wxCommandEvent);

wxBEGIN_EVENT_TABLE(ImageDataSelector, wxPanel)
    EVT_CHOICE(wxID_ANY, ImageDataSelector::OnChoiceSelected)
    EVT_BUTTON(wxID_REFRESH, ImageDataSelector::OnRefreshButton)
    EVT_BUTTON(wxID_CLEAR, ImageDataSelector::OnClearButton)
wxEND_EVENT_TABLE()

ImageDataSelector::ImageDataSelector(wxWindow* parent, wxWindowID id,
                                   const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size),
      m_repository(ImageDataRepository::GetInstance()) {
    
    // Create controls
    m_statusText = new wxStaticText(this, wxID_ANY, wxT("ImageData 목록"));
    m_imageChoice = new wxChoice(this, wxID_ANY);
    m_refreshButton = new wxButton(this, wxID_REFRESH, wxT("새로고침"));
    m_clearButton = new wxButton(this, wxID_CLEAR, wxT("모두 삭제"));
    
    // Setup layout
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    wxBoxSizer* choiceSizer = new wxBoxSizer(wxHORIZONTAL);
    choiceSizer->Add(m_imageChoice, 1, wxEXPAND | wxALL, 5);
    
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->Add(m_refreshButton, 0, wxALL, 5);
    buttonSizer->Add(m_clearButton, 0, wxALL, 5);
    
    mainSizer->Add(m_statusText, 0, wxALL | wxALIGN_CENTER, 5);
    mainSizer->Add(choiceSizer, 0, wxEXPAND | wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxALL, 5);
    
    SetSizer(mainSizer);
    
    // Initialize the list
    RefreshImageList();
}

void ImageDataSelector::RefreshImageList() {
    // Clear current items
    m_imageChoice->Clear();
    
    // Get all ImageData IDs
    std::vector<std::string> ids = m_repository.GetAllIds();
    
    // Add items to choice control
    for (const auto& id : ids) {
        std::string name = m_repository.GetImageName(id);
        std::string displayText = name + " (" + id + ")";
        m_imageChoice->Append(wxString::FromUTF8(displayText.c_str()), new wxStringClientData(id));
    }
    
    // Update status
    std::stringstream ss;
    ss << "ImageData 목록 (" << ids.size() << "개)";
    m_statusText->SetLabel(wxString::FromUTF8(ss.str().c_str()));
    
    // Enable/disable buttons based on availability
    m_clearButton->Enable(!ids.empty());
    m_imageChoice->Enable(!ids.empty());
}

std::string ImageDataSelector::GetSelectedId() const {
    int selection = m_imageChoice->GetSelection();
    if (selection != wxNOT_FOUND) {
        wxStringClientData* clientData = dynamic_cast<wxStringClientData*>(m_imageChoice->GetClientObject(selection));
        if (clientData) {
            return clientData->GetData().ToStdString();
        }
    }
    return "";
}

void ImageDataSelector::SetSelectedId(const std::string& id) {
    for (int i = 0; i < m_imageChoice->GetCount(); ++i) {
        wxStringClientData* clientData = dynamic_cast<wxStringClientData*>(m_imageChoice->GetClientObject(i));
        if (clientData && clientData->GetData().ToStdString() == id) {
            m_imageChoice->SetSelection(i);
            break;
        }
    }
}

void ImageDataSelector::OnChoiceSelected(wxCommandEvent& event) {
    OnSelectionChanged();
}

void ImageDataSelector::OnRefreshButton(wxCommandEvent& event) {
    RefreshImageList();
}

void ImageDataSelector::OnClearButton(wxCommandEvent& event) {
    // Get all IDs and remove them
    std::vector<std::string> ids = m_repository.GetAllIds();
    for (const auto& id : ids) {
        m_repository.RemoveImageData(id);
    }
    
    RefreshImageList();
    OnSelectionChanged();
}

void ImageDataSelector::OnSelectionChanged() {
    // Notify parent of selection change
    wxCommandEvent evt(ImageDataSelectionChangedEvent, GetId());
    evt.SetEventObject(this);
    GetParent()->GetEventHandler()->ProcessEvent(evt);
}
