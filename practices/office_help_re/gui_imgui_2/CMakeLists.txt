cmake_minimum_required(VERSION 3.30)

# 1. vcpkg 경로 설정 (본인의 경로로 수정하세요)
set(VCPKG_PATH "C:/vcpkg") 
# 2. 핵심 설정: project() 호출 전에 선언해야 vcpkg가 올바르게 작동합니다.
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
endif()

# 4. 정적 런타임(/MT) 강제 설정 (정적 라이브러리 사용 시 필수)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


project(ImGuiExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ImGui using vcpkg
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
# Add executable
add_executable(${PROJECT_NAME} main.cpp source_loader.cpp)

# Windows specific settings
if(WIN32)
    target_link_libraries(${PROJECT_NAME} 
        PRIVATE
        imgui::imgui
        glfw
        ${OpenCV_LIBS}
        OpenGL::GL
    )
endif()

# Add compile definition for ImGui
target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_ENABLED)