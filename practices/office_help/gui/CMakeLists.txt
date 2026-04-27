cmake_minimum_required(VERSION 3.28)

# set(VCPKG_TARGET_TRIPLET "x64-windows-static") # 정적 라이브러리 사용 시
# MSVC 런타임 라이브러리 일치 (매우 중요)
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # MT/MTd 설정

project(MyGuiProject LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)


# 1. 패키지 로드
find_package(wxWidgets REQUIRED COMPONENTS core base)
find_package(OpenCV REQUIRED)

# 2. 인텔리센스와 빌드를 위한 설정 로드 (빨간 줄 방지)
include(${wxWidgets_USE_FILE}) 

add_executable(gui WIN32)

# 3. 소스 및 모듈 관리
target_sources(gui 
    PRIVATE
        image_processor.cpp
        image_data.cpp
        image_data_repository.cpp
        image_edit_controller.cpp
        controller_factory.cpp
        # image_data_selector.cpp
        image_viewer_frame.cpp
        zoom_frame.cpp
        window_factory.cpp
        scrolled_image_window.cpp
        scrolled_image_window_segmentation.cpp
        image_edit_gui.cpp
    PUBLIC
    FILE_SET CXX_MODULES FILES 
        image_processor.cppm
        image_data.cppm
        image_data_repository.cppm
        image_edit_controller.cppm
        controller_factory.cppm
        utils.cppm
        # image_data_selector.cppm
        image_viewer_frame.cppm
        zoom_frame.cppm
        window_factory.cppm
        image_info_panel.cppm
        bounding_box_control_panel.cppm
        scrolled_image_window.cppm
)


# 4. 의존성 연결 (현대적인 Target 기반 방식)
# target_link_libraries만으로 헤더 경로, 라이브러리, 전처리기가 모두 전파됩니다.
target_link_libraries(gui PRIVATE 
    ${wxWidgets_LIBRARIES}
    ${OpenCV_LIBS}
)

# 5. 인클루드 경로 설정 (USE_FILE이 이미 처리하지만, OpenCV를 위해 명시)
# 주의: ${wxWidgets_USE_FILE}은 절대 여기에 넣지 마세요!
target_include_directories(gui PRIVATE ${OpenCV_INCLUDE_DIRS})

# # 6. wxWidgets 매크로 정의 적용
# target_compile_definitions(gui PRIVATE ${wxWidgets_DEFINITIONS})

# set_target_properties(gui PRIVATE ${OpenCV_INCLUDE_DIRS} PROPERTIES 
#     CXX_SCAN_FOR_MODULES ON
# )