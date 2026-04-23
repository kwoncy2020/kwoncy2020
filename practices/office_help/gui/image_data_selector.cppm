module;
#include <wx/wx.h>
#include <wx/choice.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <vector>
#include <string>

export module ImageDataSelector;

import ImageDataRepository;


// for the future, we can add more functionality to this class. if it required
export class ImageDataSelector : public wxPanel {
public:
    ImageDataSelector(wxWindow* parent, wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize);
    
    // ImageData management
    void RefreshImageList();
    std::string GetSelectedId() const;
    void SetSelectedId(const std::string& id);
    
    // Event handling
    void OnSelectionChanged();
    void OnRefreshClicked();
    void OnClearClicked();
    
    // Custom event
    wxDECLARE_EVENT(ImageDataSelectionChangedEvent, wxCommandEvent);

private:
    wxChoice* m_imageChoice;
    wxButton* m_refreshButton;
    wxButton* m_clearButton;
    wxStaticText* m_statusText;
    ImageDataRepository& m_repository;
    
    void OnChoiceSelected(wxCommandEvent& event);
    void OnRefreshButton(wxCommandEvent& event);
    void OnClearButton(wxCommandEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};

// Event declaration
wxDECLARE_EVENT(ImageDataSelectionChangedEvent, wxCommandEvent);
