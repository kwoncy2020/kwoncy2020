cmake_minimum_required(VERSION 3.30)


if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)

endif()

# 1. 자동화 도구가 Clang을 못 찾도록 컴파일러 경로를 MSVC로 박아버림
set(CMAKE_C_COMPILER "cl.exe")
set(CMAKE_CXX_COMPILER "cl.exe")

# 1. vcpkg 경로 설정 (본인의 경로로 수정하세요)
set(VCPKG_PATH "C:/vcpkg") 
# 2. 핵심 설정: project() 호출 전에 선언해야 vcpkg가 올바르게 작동합니다.
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "" FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

project(MyImGuiProject LANGUAGES CXX)
message(STATUS "현재 타겟 아키텍처: ${CMAKE_SIZEOF_VOID_P}")

set(CMAKE_CXX_STANDARD 20)
# 4. 정적 런타임(/MT) 강제 설정 (정적 라이브러리 사용 시 필수)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


# 5. 라이브러리 찾기
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenCV REQUIRED)

# 6. 헤더 디렉토리 설정
include_directories(${CMAKE_SOURCE_DIR}/include)


# 7. 소스 파일 수집
file(GLOB_RECURSE CORE_SOURCES 
    "src/core/*.cpp"
    "src/image/*.cpp"
    "src/backends/*.cpp"
    "src/sources/*.cpp"
    "src/graphics/*.cpp"
    "src/processing/*.cpp"
    "src/processing/algorithms/*.cpp"
    "src/ui/*.cpp"
    "src/utils/*.cpp"
)


# 8. 실행 파일 생성
add_executable(${PROJECT_NAME} 
    # main.cpp
    main_temp.cpp
    # "src/graphics/OpenGLBackend.cpp"
    # ${CORE_SOURCES}
)


# 9. 라이브러리 링크
target_link_libraries(${PROJECT_NAME} PRIVATE 
    imgui::imgui        # ImGui 핵심 및 백엔드 소스가 포함됨
    glfw                # GLFW 라이브러리
    ${OpenCV_LIBS}      # OpenCV 라이브러리들
    opengl32            # 윈도우 기본 OpenGL 라이브러리
)


# # ImGui 백엔드(GLFW/OpenGL3) 직접 링크 (필요한 경우)
# find_package(glfw3 CONFIG REQUIRED)
# target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
