-- 툴체인 자동 감지 기능을 완전히 끕니다.
set_policy("check.auto_toolchains", false)

-- 컴파일러 실행 파일 경로를 직접 하드코딩 (사용자님의 실제 경로로 수정)
set_config("cc",  "C:/clang+llvm-22.1.3-x86_64-pc-windows-msvc/bin/clang.exe")
set_config("cxx", "C:/clang+llvm-22.1.3-x86_64-pc-windows-msvc/bin/clang++.exe")
set_config("ld",  "C:/clang+llvm-22.1.3-x86_64-pc-windows-msvc/bin/clang++.exe")
set_config("ar",  "C:/clang+llvm-22.1.3-x86_64-pc-windows-msvc/bin/llvm-ar.exe")
set_config("sh",  "C:/clang+llvm-22.1.3-x86_64-pc-windows-msvc/bin/clang++.exe")

target("Office_help")
    set_kind("binary")
    set_languages("c++20")

    -- 중요: set_toolchains("clang") 명령어를 절대 쓰지 마세요! 
    -- 쓰면 다시 MSVC를 찾으러 떠납니다.

    -- 직접 경로 지정
    add_includedirs("C:/clang+llvm-22.1.3-x86_64-pc-windows-msvc/include")
    add_includedirs("C:/MyLibs/opencv/include")
    add_includedirs("C:/MyLibs/imgui")
    
    add_linkdirs("C:/MyLibs/opencv/lib")
    add_linkdirs("C:/src/glew/x64-windows-static/lib")

    add_links("opencv_core", "glfw3", "opengl32", "glew32")

    add_files("main.cpp")
    add_rules("plugin.compile_commands.autoupdate")