module;
#include <wx/wx.h>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

export module WindowFactory;

export enum WindowType { TYPE_VIEWER, TYPE_ZOOM_POPUP };

export class WindowManager {
public:
    static WindowManager& Get();
    wxFrame* CreateWindow(WindowType type, wxWindow* parent, const cv::Mat& data = cv::Mat());
    
    void CloseAll();

private:
    WindowManager();
    std::vector<wxFrame*> m_activeWindows;
    void OnWindowClose(wxCloseEvent& event);

};
