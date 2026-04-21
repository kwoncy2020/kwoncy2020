#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <ranges>
// #include <optional>

#ifndef UNICODE
#define UNICODE
#endif



struct WHInfo{
    HWND hwnd;
    std::wstring title;
    std::vector<std::unique_ptr<WHInfo>> children;

    WHInfo(HWND hwnd): hwnd(hwnd){
        WCHAR title_buffer[256];
        int length = GetWindowTextW(hwnd, title_buffer, 256);
        this->title = std::wstring(title_buffer, length);
    }
};

class WHManager{

    static std::vector<std::unique_ptr<WHInfo>> WH_all_infos;
    static std::unordered_map<HWND ,WHInfo*> WTH_map_by_handle;   // w top handle map
    static std::unordered_map<std::wstring ,std::vector<WHInfo*>> WTH_map_by_title; // w top handle map
    static std::vector<WHInfo*> empty_infos;  // return for empty matching without std::optional or std::expected

    // static std::unordered_map<std::wstring ,std::shared_ptr<WHInfo>> WH_map_by_title;
    static BOOL CALLBACK EnumTopLevelWindowsProc(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK EnumChildWindowsProc(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
    public:
        WHManager();
        ~WHManager();

        std::vector<WHInfo*>& get_WHInfos_with_title(std::wstring& title);
        WHInfo* get_WHInfo_with_hwnd(HWND& hwnd);
        std::vector<HWND> get_handles_with_title(std::wstring& title);
        std::wstring& get_title_with_handle(HWND& hwnd);
        std::unordered_map<HWND ,WHInfo*>& get_whole_info_map_by_handle();   // w top handle map
        std::unordered_map<std::wstring ,std::vector<WHInfo*>>& get_whole_info_map_by_title(); // w top 
};

WHManager::WHManager(){
    EnumWindows(this->EnumTopLevelWindowsProc, reinterpret_cast<LPARAM>(&WH_all_infos));
}

WHManager::~WHManager(){
    WHManager::WH_all_infos.clear();
    WHManager::WTH_map_by_handle.clear();
    WHManager::WTH_map_by_title.clear();
}


BOOL CALLBACK WHManager::EnumTopLevelWindowsProc(HWND hwnd, LPARAM lParam){
    if (!IsWindowVisible(hwnd)) return TRUE;

    std::vector<std::unique_ptr<WHInfo>>* all_windows = reinterpret_cast<std::vector<std::unique_ptr<WHInfo>>*>(lParam); // WH_all_infos == all_windows

    // WHInfo info(hwnd);
    std::unique_ptr<WHInfo> info_ptr = std::make_unique<WHInfo>(hwnd);

    EnumChildWindows(hwnd, WHManager::EnumChildWindowsProc, reinterpret_cast<LPARAM>(&(info_ptr->children)));

    all_windows->push_back(std::move(info_ptr));
    WHInfo* last_wh_ptr = all_windows->back().get();
    WTH_map_by_handle[last_wh_ptr->hwnd] = last_wh_ptr;
    WTH_map_by_title[last_wh_ptr->title].push_back(last_wh_ptr);    // warning : same title or empty title
    
    return TRUE;
}


BOOL CALLBACK WHManager::EnumChildWindowsProc(HWND hwnd, LPARAM lParam){
    if (!IsWindowVisible(hwnd)) return TRUE;

    std::vector<std::unique_ptr<WHInfo>>* child_windows = reinterpret_cast<std::vector<std::unique_ptr<WHInfo>>*>(lParam);  // parent_info.children == child_windows

    // WHInfo info(hwnd);
    std::unique_ptr<WHInfo> info_ptr = std::make_unique<WHInfo>(hwnd);
    child_windows->push_back(std::move(info_ptr));
    
    WHInfo* last_wh_ptr = child_windows->back().get();
    WTH_map_by_handle[last_wh_ptr->hwnd] = last_wh_ptr;
    WTH_map_by_title[last_wh_ptr->title].push_back(last_wh_ptr);    // warning : same title or empty title
    
    return TRUE;
}

std::vector<WHInfo*>& WHManager::get_WHInfos_with_title(std::wstring& title){
    if (WTH_map_by_title.count(title)) return WTH_map_by_title[title];  

    return empty_infos;
}

WHInfo* WHManager::get_WHInfo_with_hwnd(HWND& hwnd){
    if (WTH_map_by_handle.count(hwnd)) return WTH_map_by_handle[hwnd];
    
    return nullptr;
}

std::vector<HWND> WHManager::get_handles_with_title(std::wstring& title){
    return this->get_WHInfos_with_title(title) | std::views::transform([](const auto& WHInfo_ptr){return WHInfo_ptr->hwnd;}) | std::ranges::to<std::vector<HWND>>();
}

std::wstring& WHManager::get_title_with_handle(HWND& hwnd){
    return (this->get_WHInfo_with_hwnd(hwnd))->title;
}

std::unordered_map<HWND ,WHInfo*>& WHManager::get_whole_info_map_by_handle(){
    return this->WTH_map_by_handle;
};  
std::unordered_map<std::wstring ,std::vector<WHInfo*>>& WHManager::get_whole_info_map_by_title(){
    return this->WTH_map_by_title;
}; 


std::vector<std::unique_ptr<WHInfo>> WHManager::WH_all_infos;

std::unordered_map<HWND ,WHInfo*> WHManager::WTH_map_by_handle;

std::unordered_map<std::wstring ,std::vector<WHInfo*>> WHManager::WTH_map_by_title; 

std::vector<WHInfo*> WHManager::empty_infos; // empty_infos는 빈 벡터로 초기화

int main(){
    WHManager wh_manager;

    auto& map_b_handle = wh_manager.get_whole_info_map_by_handle();
    auto& map_b_title = wh_manager.get_whole_info_map_by_title();

    auto a = map_b_handle.size();
    auto b = map_b_title.size();

    return 0;
}