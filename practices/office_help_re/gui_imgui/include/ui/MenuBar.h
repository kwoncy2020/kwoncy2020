#pragma once

#include <string>
#include <functional>
#include <memory>

/**
 * @brief 메뉴 바 UI 패널 클래스
 * 
 * 이 클래스는 애플리케이션의 메뉴 바를 관리합니다.
 * 파일, 편집, 보기, 도구, 도움말 등의 메뉴를 제공합니다.
 * 
 * 주요 기능:
 * - 다양한 메뉴 항목 제공
 * - 메뉴 이벤트 콜백 처리
 * - 단축키 지원
 * - 동적 메뉴 업데이트
 */
class MenuBar {
public:
    /**
     * @brief 메뉴 이벤트 콜백 타입
     */
    using MenuCallback = std::function<void()>;

private:
    // 파일 메뉴 콜백들
    MenuCallback m_openFileCallback;
    MenuCallback m_saveFileCallback;
    MenuCallback m_saveAsFileCallback;
    MenuCallback m_exitCallback;
    
    // 편집 메뉴 콜백들
    MenuCallback m_undoCallback;
    MenuCallback m_redoCallback;
    MenuCallback m_copyCallback;
    MenuCallback m_pasteCallback;
    MenuCallback m_deleteCallback;
    
    // 보기 메뉴 콜백들
    MenuCallback m_zoomInCallback;
    MenuCallback m_zoomOutCallback;
    MenuCallback m_zoomResetCallback;
    MenuCallback m_fitToWindowCallback;
    MenuCallback m_actualSizeCallback;
    
    // 도구 메뉴 콜백들
    MenuCallback m_boundingBoxDetectionCallback;
    MenuCallback m_segmentationCallback;
    MenuCallback m_matchingCallback;
    MenuCallback m_processingPipelineCallback;
    
    // 도움말 메뉴 콜백들
    MenuCallback m_aboutCallback;
    MenuCallback m_helpCallback;
    
    // 메뉴 상태
    bool m_fileMenuEnabled;
    bool m_editMenuEnabled;
    bool m_viewMenuEnabled;
    bool m_toolsMenuEnabled;
    bool m_helpMenuEnabled;
    
    // 메뉴 활성화 상태
    bool m_canUndo;
    bool m_canRedo;
    bool m_hasSelection;
    bool m_hasImage;
    bool m_isProcessing;

public:
    /**
     * @brief 생성자
     */
    MenuBar();
    
    /**
     * @brief 소멸자
     */
    ~MenuBar() = default;

    // 메뉴 렌더링
    void render();

    // 파일 메뉴 콜백 설정
    void setOpenFileCallback(MenuCallback callback);
    void setSaveFileCallback(MenuCallback callback);
    void setSaveAsFileCallback(MenuCallback callback);
    void setExitCallback(MenuCallback callback);
    
    // 편집 메뉴 콜백 설정
    void setUndoCallback(MenuCallback callback);
    void setRedoCallback(MenuCallback callback);
    void setCopyCallback(MenuCallback callback);
    void setPasteCallback(MenuCallback callback);
    void setDeleteCallback(MenuCallback callback);
    
    // 보기 메뉴 콜백 설정
    void setZoomInCallback(MenuCallback callback);
    void setZoomOutCallback(MenuCallback callback);
    void setZoomResetCallback(MenuCallback callback);
    void setFitToWindowCallback(MenuCallback callback);
    void setActualSizeCallback(MenuCallback callback);
    
    // 도구 메뉴 콜백 설정
    void setBoundingBoxDetectionCallback(MenuCallback callback);
    void setSegmentationCallback(MenuCallback callback);
    void setMatchingCallback(MenuCallback callback);
    void setProcessingPipelineCallback(MenuCallback callback);
    
    // 도움말 메뉴 콜백 설정
    void setAboutCallback(MenuCallback callback);
    void setHelpCallback(MenuCallback callback);

    // 메뉴 활성화 상태 설정
    void setFileMenuEnabled(bool enabled);
    void setEditMenuEnabled(bool enabled);
    void setViewMenuEnabled(bool enabled);
    void setToolsMenuEnabled(bool enabled);
    void setHelpMenuEnabled(bool enabled);
    
    // 편집 상태 설정
    void setCanUndo(bool canUndo);
    void setCanRedo(bool canRedo);
    void setHasSelection(bool hasSelection);
    void setHasImage(bool hasImage);
    void setIsProcessing(bool isProcessing);

private:
    /**
     * @brief 파일 메뉴 렌더링
     */
    void renderFileMenu();
    
    /**
     * @brief 편집 메뉴 렌더링
     */
    void renderEditMenu();
    
    /**
     * @brief 보기 메뉴 렌더링
     */
    void renderViewMenu();
    
    /**
     * @brief 도구 메뉴 렌더링
     */
    void renderToolsMenu();
    
    /**
     * @brief 도움말 메뉴 렌더링
     */
    void renderHelpMenu();
    
    /**
     * @brief 메뉴 항목 렌더링 헬퍼
     * @param label 메뉴 레이블
     * @param shortcut 단축키
     * @param enabled 활성화 여부
     * @param callback 콜백 함수
     * @return 메뉴 선택 여부
     */
    bool renderMenuItem(const char* label, const char* shortcut, bool enabled, MenuCallback callback);
    
    /**
     * @brief 구분선 렌더링
     */
    void renderSeparator();
    
    /**
     * @brief 체크박스 메뉴 항목 렌더링
     * @param label 메뉴 레이블
     * @param checked 체크 상태
     * @param enabled 활성화 여부
     * @return 체크 상태 변경 여부
     */
    bool renderCheckboxMenuItem(const char* label, bool* checked, bool enabled = true);
};
