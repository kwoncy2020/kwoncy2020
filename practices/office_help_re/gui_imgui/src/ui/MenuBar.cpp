#include "ui/MenuBar.h"
#include "imgui.h"

MenuBar::MenuBar() 
    : m_fileMenuEnabled(true), m_editMenuEnabled(true), m_viewMenuEnabled(true),
      m_toolsMenuEnabled(true), m_helpMenuEnabled(true),
      m_canUndo(false), m_canRedo(false), m_hasSelection(false),
      m_hasImage(false), m_isProcessing(false) {
}

void MenuBar::render() {
    if (ImGui::BeginMainMenuBar()) {
        renderFileMenu();
        renderEditMenu();
        renderViewMenu();
        renderToolsMenu();
        renderHelpMenu();
        
        ImGui::EndMainMenuBar();
    }
}

// 파일 메뉴 콜백 설정
void MenuBar::setOpenFileCallback(MenuCallback callback) {
    m_openFileCallback = callback;
}

void MenuBar::setSaveFileCallback(MenuCallback callback) {
    m_saveFileCallback = callback;
}

void MenuBar::setSaveAsFileCallback(MenuCallback callback) {
    m_saveAsFileCallback = callback;
}

void MenuBar::setExitCallback(MenuCallback callback) {
    m_exitCallback = callback;
}

// 편집 메뉴 콜백 설정
void MenuBar::setUndoCallback(MenuCallback callback) {
    m_undoCallback = callback;
}

void MenuBar::setRedoCallback(MenuCallback callback) {
    m_redoCallback = callback;
}

void MenuBar::setCopyCallback(MenuCallback callback) {
    m_copyCallback = callback;
}

void MenuBar::setPasteCallback(MenuCallback callback) {
    m_pasteCallback = callback;
}

void MenuBar::setDeleteCallback(MenuCallback callback) {
    m_deleteCallback = callback;
}

// 보기 메뉴 콜백 설정
void MenuBar::setZoomInCallback(MenuCallback callback) {
    m_zoomInCallback = callback;
}

void MenuBar::setZoomOutCallback(MenuCallback callback) {
    m_zoomOutCallback = callback;
}

void MenuBar::setZoomResetCallback(MenuCallback callback) {
    m_zoomResetCallback = callback;
}

void MenuBar::setFitToWindowCallback(MenuCallback callback) {
    m_fitToWindowCallback = callback;
}

void MenuBar::setActualSizeCallback(MenuCallback callback) {
    m_actualSizeCallback = callback;
}

// 도구 메뉴 콜백 설정
void MenuBar::setBoundingBoxDetectionCallback(MenuCallback callback) {
    m_boundingBoxDetectionCallback = callback;
}

void MenuBar::setSegmentationCallback(MenuCallback callback) {
    m_segmentationCallback = callback;
}

void MenuBar::setMatchingCallback(MenuCallback callback) {
    m_matchingCallback = callback;
}

void MenuBar::setProcessingPipelineCallback(MenuCallback callback) {
    m_processingPipelineCallback = callback;
}

// 도움말 메뉴 콜백 설정
void MenuBar::setAboutCallback(MenuCallback callback) {
    m_aboutCallback = callback;
}

void MenuBar::setHelpCallback(MenuCallback callback) {
    m_helpCallback = callback;
}

// 메뉴 활성화 상태 설정
void MenuBar::setFileMenuEnabled(bool enabled) {
    m_fileMenuEnabled = enabled;
}

void MenuBar::setEditMenuEnabled(bool enabled) {
    m_editMenuEnabled = enabled;
}

void MenuBar::setViewMenuEnabled(bool enabled) {
    m_viewMenuEnabled = enabled;
}

void MenuBar::setToolsMenuEnabled(bool enabled) {
    m_toolsMenuEnabled = enabled;
}

void MenuBar::setHelpMenuEnabled(bool enabled) {
    m_helpMenuEnabled = enabled;
}

// 편집 상태 설정
void MenuBar::setCanUndo(bool canUndo) {
    m_canUndo = canUndo;
}

void MenuBar::setCanRedo(bool canRedo) {
    m_canRedo = canRedo;
}

void MenuBar::setHasSelection(bool hasSelection) {
    m_hasSelection = hasSelection;
}

void MenuBar::setHasImage(bool hasImage) {
    m_hasImage = hasImage;
}

void MenuBar::setIsProcessing(bool isProcessing) {
    m_isProcessing = isProcessing;
}

// Private 메서드

void MenuBar::renderFileMenu() {
    if (!m_fileMenuEnabled) return;
    
    if (ImGui::BeginMenu("File")) {
        renderMenuItem("Open", "Ctrl+O", true, m_openFileCallback);
        renderMenuItem("Save", "Ctrl+S", m_hasImage, m_saveFileCallback);
        renderMenuItem("Save As...", "Ctrl+Shift+S", m_hasImage, m_saveAsFileCallback);
        renderSeparator();
        renderMenuItem("Exit", "Alt+F4", true, m_exitCallback);
        
        ImGui::EndMenu();
    }
}

void MenuBar::renderEditMenu() {
    if (!m_editMenuEnabled) return;
    
    if (ImGui::BeginMenu("Edit")) {
        renderMenuItem("Undo", "Ctrl+Z", m_canUndo, m_undoCallback);
        renderMenuItem("Redo", "Ctrl+Y", m_canRedo, m_redoCallback);
        renderSeparator();
        renderMenuItem("Copy", "Ctrl+C", m_hasSelection, m_copyCallback);
        renderMenuItem("Paste", "Ctrl+V", true, m_pasteCallback);
        renderMenuItem("Delete", "Del", m_hasSelection, m_deleteCallback);
        
        ImGui::EndMenu();
    }
}

void MenuBar::renderViewMenu() {
    if (!m_viewMenuEnabled) return;
    
    if (ImGui::BeginMenu("View")) {
        renderMenuItem("Zoom In", "Ctrl++", m_hasImage, m_zoomInCallback);
        renderMenuItem("Zoom Out", "Ctrl+-", m_hasImage, m_zoomOutCallback);
        renderMenuItem("Reset Zoom", "Ctrl+0", m_hasImage, m_zoomResetCallback);
        renderSeparator();
        renderMenuItem("Fit to Window", "Ctrl+F", m_hasImage, m_fitToWindowCallback);
        renderMenuItem("Actual Size", "Ctrl+1", m_hasImage, m_actualSizeCallback);
        
        ImGui::EndMenu();
    }
}

void MenuBar::renderToolsMenu() {
    if (!m_toolsMenuEnabled) return;
    
    if (ImGui::BeginMenu("Tools")) {
        renderMenuItem("Bounding Box Detection", "", !m_isProcessing, m_boundingBoxDetectionCallback);
        renderMenuItem("Segmentation", "", !m_isProcessing, m_segmentationCallback);
        renderMenuItem("Image Matching", "", !m_isProcessing, m_matchingCallback);
        renderSeparator();
        renderMenuItem("Processing Pipeline", "", !m_isProcessing, m_processingPipelineCallback);
        
        ImGui::EndMenu();
    }
}

void MenuBar::renderHelpMenu() {
    if (!m_helpMenuEnabled) return;
    
    if (ImGui::BeginMenu("Help")) {
        renderMenuItem("Help", "F1", true, m_helpCallback);
        renderSeparator();
        renderMenuItem("About", "", true, m_aboutCallback);
        
        ImGui::EndMenu();
    }
}

bool MenuBar::renderMenuItem(const char* label, const char* shortcut, bool enabled, MenuCallback callback) {
    bool selected = false;
    
    if (ImGui::MenuItem(label, shortcut, false, enabled)) {
        selected = true;
        if (callback) {
            callback();
        }
    }
    
    return selected;
}

void MenuBar::renderSeparator() {
    ImGui::Separator();
}

bool MenuBar::renderCheckboxMenuItem(const char* label, bool* checked, bool enabled) {
    return ImGui::MenuItem(label, nullptr, checked, enabled);
}
