# GUI Build Plan

## Project Overview
GUI application using wxWidgets and OpenCV with C++20 modules system. The application provides image processing capabilities with a main interface, image viewer, and zoom functionality.

## Current Build Issues Analysis

### 1. CMakeLists.txt Issues
**Problem**: Missing source file in build configuration
- `image_processor.cpp` is not included in the target sources
- This will cause linker errors when ImageProcessor::GetZoomedROI is called

**Solution**: Add `image_processor.cpp` to PRIVATE sources section

### 2. Module System Issues
**Problem**: Inconsistent module declarations and imports
- `image_viewer_frame.cpp` imports WindowFactory but doesn't declare as module
- `zoom_frame.cpp` doesn't participate in module system
- `image_processor.cpp` is not a module but is used by modules

**Solution**: 
- Make `image_processor.cpp` a proper module or provide header interface
- Ensure all files that use modules have proper module declarations

### 3. Missing Class Declarations
**Problem**: `window_factory.cpp` references classes that aren't declared
- `ImageViewer` class is used but not declared/included
- `ZoomFrame` class is used but not declared/included

**Solution**: Add proper forward declarations or includes

### 4. API Inconsistency
**Problem**: Method calls don't match available APIs
- `image_viewer_frame.cpp` calls `WindowFactory::CreateChildWindow()` which doesn't exist
- Should use `WindowManager::CreateWindow()` instead

**Solution**: Fix method call to use correct API

### 5. Missing Utility Functions
**Problem**: Required utility functions are not implemented
- `ConvertMatToBitmap()` function is referenced but not implemented
- This is needed for OpenCV Mat to wxBitmap conversion

**Solution**: Implement the conversion utility function

## Detailed Fix Plan

### File: CMakeLists.txt
```cmake
# Add image_processor.cpp to PRIVATE sources:
target_sources(gui 
    PUBLIC
    FILE_SET CXX_MODULES FILES 
        window_factory.cppm
    PRIVATE
        image_edit_gui.cpp
        window_factory.cpp
        image_viewer_frame.cpp
        zoom_frame.cpp
        image_processor.cpp  # <-- ADD THIS
)
```

### File: window_factory.cpp
- Add forward declarations for ImageViewer and ZoomFrame classes
- Or include appropriate headers

### File: image_viewer_frame.cpp
- Add proper module declaration
- Fix `WindowFactory::CreateChildWindow` to `WindowManager::CreateWindow`
- Add missing utility function implementation

### File: zoom_frame.cpp
- Implement `ConvertMatToBitmap` function
- Consider integrating with module system

### File: image_processor.cpp
- Convert to module or provide header interface
- Ensure proper integration with module system

## Implementation Priority
1. **High**: Fix CMakeLists.txt to include all source files
2. **High**: Resolve module declaration issues
3. **High**: Fix missing class declarations
4. **High**: Fix API method calls
5. **Medium**: Implement missing utility functions

## Testing Strategy
After fixes:
1. Build should complete without errors
2. Application should launch and show main interface
3. Image viewer should open from main interface
4. Zoom functionality should work when hovering over images

## Dependencies
- wxWidgets (core, base components)
- OpenCV libraries
- C++20 compiler with module support
- CMake 3.28+
