module;
#include <wx/wx.h>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

module WindowFactory;

import ImageViewerFrame;
import ZoomFrame;

WindowManager::WindowManager() {
    // 필요한 초기화 코드가 없다면 비워두어도 됩니다.
}

WindowManager& WindowManager::Get() {
    static WindowManager instance;
    return instance;
}

wxFrame* WindowManager::CreateWindow(WindowType type, wxWindow* parent, const cv::Mat& data) {
            wxFrame* frame = nullptr;
    
            if (type == TYPE_VIEWER) {
                frame = new ImageViewerFrame(parent, data);
            } 
            else if (type == TYPE_ZOOM_POPUP) {
                frame = new ZoomFrame(parent);
            }
    
            if (frame) {
                m_activeWindows.push_back(frame);
                // 창이 닫힐 때 리스트에서 제거하기 위한 이벤트 연결
                frame->Bind(wxEVT_CLOSE_WINDOW, &WindowManager::OnWindowClose, this);
            }
            return frame;
        }


void WindowManager::CloseAll() {
    for (auto* win : m_activeWindows) {
        if (win) win->Destroy(); // wxWidgets에서는 delete 대신 Destroy 권장
    }
    m_activeWindows.clear();
}

void WindowManager::OnWindowClose(wxCloseEvent& event) {
    wxFrame* win = static_cast<wxFrame*>(event.GetEventObject());
    // 리스트에서 제거
    m_activeWindows.erase(
        std::remove(m_activeWindows.begin(), m_activeWindows.end(), win), 
        m_activeWindows.end()
    );
    event.Skip(); // wxWidgets이 실제 창을 닫도록 허용
}
