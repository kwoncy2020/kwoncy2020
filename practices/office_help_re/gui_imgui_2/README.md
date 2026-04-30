# ImGui Example Project

## Setup Instructions

1. **Download ImGui:**
   - Go to https://github.com/ocornut/imgui/releases
   - Download the latest release (e.g., imgui-1.90.8.zip)
   - Extract it to this folder, rename the extracted folder to `imgui`

2. **Build the project:**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. **Run the application:**
   ```bash
   .\ImGuiExample.exe
   ```

## Project Structure
```
gui_imgui_2/
  main.cpp              # Main application code
  CMakeLists.txt        # Build configuration
  imgui/                # ImGui library (downloaded separately)
    imgui.cpp
    imgui.h
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    backends/
      imgui_impl_win32.cpp
      imgui_impl_win32.h
      imgui_impl_dx11.cpp
      imgui_impl_dx11.h
```

## Features
- Basic ImGui window with demo
- Interactive controls (sliders, buttons, color picker)
- DirectX 11 rendering backend
- Windows native integration
