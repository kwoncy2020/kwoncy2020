workspace "OfficeProject"
    configurations { "RelWithDebInfo" }
    platforms { "x64" }

project "Office_help"
    kind "ConsoleApp"
    language "C++"
    cppvariant "clang"
    
    -- 1. 컴파일러 설정
    toolset "clang"
    buildoptions { "-std=c++20", "-O2", "-g", "-static" }

    -- 2. 경로 설정 (사용자님의 실제 경로로 수정)
    local opencv_path = "C:/opencv/install"
    includedirs { opencv_path .. "/include" }
    libdirs     { opencv_path .. "/x64/vc17/staticlib" }

    -- 3. [핵심] 라이브러리 목록 자동 생성
    -- staticlib 폴더 안의 모든 .lib 파일을 찾아서 링크에 추가합니다.
    links { os.matchfiles(opencv_path .. "/x64/vc17/staticlib/*.lib") }
    
    -- 4. 윈도우 시스템 기본 라이브러리 (정적 빌드 필수 의존성)
    links { "user32", "gdi32", "comdlg32", "advapi32", "shell32", "vfw32", "cap" }

    files { "main.cpp" }