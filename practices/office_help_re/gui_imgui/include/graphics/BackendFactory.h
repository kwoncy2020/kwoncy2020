#pragma once

#include "core/IGraphicsBackend.h"
#include <memory>
#include <string>

/**
 * @brief 그래픽 백엔드 팩토리 클래스
 * 
 * 이 클래스는 다양한 그래픽 백엔드를 생성하고 관리하기 위한
 * 팩토리 패턴을 구현합니다.
 * 
 * 주요 기능:
 * - 런타임 백엔드 선택
 * - 자동 백엔드 감지
 * - 백엔드 생성 및 초기화
 */
class BackendFactory {
public:
    /**
     * @brief 그래픽 백엔드 생성
     * @param type 생성할 백엔드 타입
     * @return 생성된 백엔드 인스턴스
     */
    static std::unique_ptr<IGraphicsBackend> createBackend(GraphicsBackendType type);
    
    /**
     * @brief 기본 백엔드 생성 (시스템에 맞게)
     * @return 생성된 백엔드 인스턴스
     */
    static std::unique_ptr<IGraphicsBackend> createDefaultBackend();
    
    /**
     * @brief 사용 가능한 백엔드 목록 가져오기
     * @return 사용 가능한 백엔드 타입 목록
     */
    static std::vector<GraphicsBackendType> getAvailableBackends();
    
    /**
     * @brief 백엔드 타입을 문자열로 변환
     * @param type 백엔드 타입
     * @return 백엔드 이름 문자열
     */
    static std::string backendTypeToString(GraphicsBackendType type);
    
    /**
     * @brief 문자열을 백엔드 타입으로 변환
     * @param name 백엔드 이름 문자열
     * @return 백엔드 타입 (변환 실패 시 기본값)
     */
    static GraphicsBackendType stringToBackendType(const std::string& name);
    
    /**
     * @brief 현재 시스템에서 권장하는 백엔드 가져오기
     * @return 권장 백엔드 타입
     */
    static GraphicsBackendType getRecommendedBackend();
    
    /**
     * @brief 특정 백엔드 사용 가능 여부 확인
     * @param type 확인할 백엔드 타입
     * @return 사용 가능 여부
     */
    static bool isBackendAvailable(GraphicsBackendType type);

private:
    /**
     * @brief OpenGL 백엔드 사용 가능 여부 확인
     * @return 사용 가능 여부
     */
    static bool isOpenGLAvailable();
    
    /**
     * @brief Vulkan 백엔드 사용 가능 여부 확인
     * @return 사용 가능 여부
     */
    static bool isVulkanAvailable();
    
    /**
     * @brief DirectX 백엔드 사용 가능 여부 확인
     * @return 사용 가능 여부
     */
    static bool isDirectXAvailable();
    
    /**
     * @brief Metal 백엔드 사용 가능 여부 확인
     * @return 사용 가능 여부
     */
    static bool isMetalAvailable();
};
