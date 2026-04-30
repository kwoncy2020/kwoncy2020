#include "graphics/BackendFactory.h"
#include "graphics/OpenGLBackend.h"
#include <iostream>
#include <algorithm>

// 플랫폼별 헤더 포함
#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef __APPLE__
    #include <TargetConditionals.h>
    #ifdef TARGET_OS_MAC
        #include <OpenGL/OpenGL.h>
    #endif
#endif

std::unique_ptr<IGraphicsBackend> BackendFactory::createBackend(GraphicsBackendType type) {
    switch (type) {
        case GraphicsBackendType::OpenGL:
            return std::make_unique<OpenGLBackend>();
            
        case GraphicsBackendType::Vulkan:
            // TODO: VulkanBackend 구현
            std::cerr << "Vulkan backend not yet implemented" << std::endl;
            return nullptr;
            
        case GraphicsBackendType::DirectX:
            // TODO: DirectXBackend 구현
            std::cerr << "DirectX backend not yet implemented" << std::endl;
            return nullptr;
            
        case GraphicsBackendType::Metal:
            // TODO: MetalBackend 구현
            std::cerr << "Metal backend not yet implemented" << std::endl;
            return nullptr;
            
        default:
            std::cerr << "Unknown graphics backend type" << std::endl;
            return nullptr;
    }
}

std::unique_ptr<IGraphicsBackend> BackendFactory::createDefaultBackend() {
    GraphicsBackendType recommended = getRecommendedBackend();
    
    if (isBackendAvailable(recommended)) {
        return createBackend(recommended);
    }
    
    // 권장 백엔드가 없으면 OpenGL 시도
    if (isBackendAvailable(GraphicsBackendType::OpenGL)) {
        std::cout << "Using OpenGL as fallback backend" << std::endl;
        return createBackend(GraphicsBackendType::OpenGL);
    }
    
    std::cerr << "No suitable graphics backend available" << std::endl;
    return nullptr;
}

std::vector<GraphicsBackendType> BackendFactory::getAvailableBackends() {
    std::vector<GraphicsBackendType> available;
    
    if (isBackendAvailable(GraphicsBackendType::OpenGL)) {
        available.push_back(GraphicsBackendType::OpenGL);
    }
    
    if (isBackendAvailable(GraphicsBackendType::Vulkan)) {
        available.push_back(GraphicsBackendType::Vulkan);
    }
    
    if (isBackendAvailable(GraphicsBackendType::DirectX)) {
        available.push_back(GraphicsBackendType::DirectX);
    }
    
    if (isBackendAvailable(GraphicsBackendType::Metal)) {
        available.push_back(GraphicsBackendType::Metal);
    }
    
    return available;
}

std::string BackendFactory::backendTypeToString(GraphicsBackendType type) {
    switch (type) {
        case GraphicsBackendType::OpenGL:
            return "OpenGL";
        case GraphicsBackendType::Vulkan:
            return "Vulkan";
        case GraphicsBackendType::DirectX:
            return "DirectX";
        case GraphicsBackendType::Metal:
            return "Metal";
        default:
            return "Unknown";
    }
}

GraphicsBackendType BackendFactory::stringToBackendType(const std::string& name) {
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    if (lowerName == "opengl" || lowerName == "gl") {
        return GraphicsBackendType::OpenGL;
    } else if (lowerName == "vulkan" || lowerName == "vk") {
        return GraphicsBackendType::Vulkan;
    } else if (lowerName == "directx" || lowerName == "dx" || lowerName == "d3d") {
        return GraphicsBackendType::DirectX;
    } else if (lowerName == "metal") {
        return GraphicsBackendType::Metal;
    }
    
    // 기본값은 OpenGL
    return GraphicsBackendType::OpenGL;
}

GraphicsBackendType BackendFactory::getRecommendedBackend() {
    // 플랫폼별 권장 백엔드
    
    #ifdef __APPLE__
        // macOS에서는 Metal 권장, 다음은 OpenGL
        if (isMetalAvailable()) {
            return GraphicsBackendType::Metal;
        }
        return GraphicsBackendType::OpenGL;
    #endif
    
    #ifdef _WIN32
        // Windows에서는 DirectX 권장, 다음은 OpenGL
        if (isDirectXAvailable()) {
            return GraphicsBackendType::DirectX;
        }
        return GraphicsBackendType::OpenGL;
    #endif
    
    // Linux 및 기타 플랫폼에서는 Vulkan 권장, 다음은 OpenGL
    if (isVulkanAvailable()) {
        return GraphicsBackendType::Vulkan;
    }
    
    return GraphicsBackendType::OpenGL;
}

bool BackendFactory::isBackendAvailable(GraphicsBackendType type) {
    switch (type) {
        case GraphicsBackendType::OpenGL:
            return isOpenGLAvailable();
        case GraphicsBackendType::Vulkan:
            return isVulkanAvailable();
        case GraphicsBackendType::DirectX:
            return isDirectXAvailable();
        case GraphicsBackendType::Metal:
            return isMetalAvailable();
        default:
            return false;
    }
}

// Private 메서드

bool BackendFactory::isOpenGLAvailable() {
    // GLFW를 통해 OpenGL 사용 가능 여부 확인
    if (!glfwInit()) {
        return false;
    }
    
    // 기본 OpenGL 버전 요구사항 확인
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // 보이지 않는 창으로 테스트
    
    GLFWwindow* testWindow = glfwCreateWindow(1, 1, "OpenGL Test", nullptr, nullptr);
    bool available = (testWindow != nullptr);
    
    if (testWindow) {
        glfwMakeContextCurrent(testWindow);
        
        // OpenGL 3.3 이상 확인
        const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        if (version) {
            std::string versionStr(version);
            // 버전 문자열 파싱 (예: "3.3.0 Core Profile")
            if (versionStr.find("3.") == std::string::npos && versionStr.find("4.") == std::string::npos) {
                available = false;
            }
        }
        
        glfwDestroyWindow(testWindow);
    }
    
    glfwTerminate();
    glfwInit(); // 다시 초기화 (다른 코드에서 사용하도록)
    
    return available;
}

bool BackendFactory::isVulkanAvailable() {
    // TODO: Vulkan 사용 가능 여부 확인
    // vkGetInstanceProcAddr 등으로 확인 가능
    return false;
}

bool BackendFactory::isDirectXAvailable() {
    #ifdef _WIN32
        // Windows에서 DirectX 사용 가능 여부 확인
        // Direct3D 11 이상 확인 가능
        return true;
    #else
        return false;
    #endif
}

bool BackendFactory::isMetalAvailable() {
    #ifdef __APPLE__
        #ifdef TARGET_OS_MAC
            // macOS에서 Metal 사용 가능 여부 확인
            // macOS 10.11 이상에서 Metal 사용 가능
            return true;
        #else
            return false;
        #endif
    #else
        return false;
    #endif
}
