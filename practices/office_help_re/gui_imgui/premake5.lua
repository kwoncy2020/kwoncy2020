workspace "MyProject"
    configurations { "Debug", "Release" }
    language "C++"
    cppvariant "clang" -- 컴파일러 종류만 명시

project "Office_help"
    kind "ConsoleApp"
    targetdir "bin"

    -- 1. 컴파일러 및 도구 경로 직접 지정 (절대경로)
    toolset "clang"
    buildoptions { "-std=c++20" }

    -- 2. 인클루드 경로 (그냥 일일이 적으세요)
    includedirs {
        "C:/clang+llvm-22.1.3-x86_64-pc-windows-msvc/include",
        "C:/MyLibs/opencv/include",
        "C:/MyLibs/imgui"
    }

    -- 3. 라이브러리 경로
    libdirs {
        "C:/MyLibs/opencv/lib",
        "C:/src/glew/x64-windows-static/lib"
    }

    -- 4. 링크할 파일 이름
    links { "opencv_core", "glfw3", "opengl32" }

    -- 5. 소스 파일
    files { "main.cpp" }