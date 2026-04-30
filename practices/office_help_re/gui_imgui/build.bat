@echo off
:: 1. 컴파일러 경로 직접 지정
set CXX="C:/clang+llvm-22.1.3-x86_64-pc-windows-msvc/bin/clang++.exe"

:: 2. 인클루드 경로 (직접 추가하세요)
set INCLUDES=-I"C:/clang+llvm-22.1.3-x86_64-pc-windows-msvc/include" ^
             -I"C:/MyLibs/opencv/include" ^
             -I"C:/MyLibs/imgui"

:: 3. 라이브러리 경로 및 링크
set LIBS=-L"C:/MyLibs/opencv/lib" ^
         -L"C:/src/glew/x64-windows-static/lib" ^
         -lopencv_core -lglfw3 -lopengl32 -lglew32

:: 4. 빌드 실행
echo 빌드를 시작합니다...
%CXX% -std=c++20 main.cpp %INCLUDES% %LIBS% -o Office_help.exe

if %errorlevel% equ 0 (
    echo [성공] Office_help.exe 가 생성되었습니다.
    Office_help.exe
) else (
    echo [실패] 에러를 확인하세요.
)
pause