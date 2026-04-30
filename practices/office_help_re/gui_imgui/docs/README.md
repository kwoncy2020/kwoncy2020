# GUI ImGui Image Processing Tool

## Overview
This is a comprehensive GUI application for image processing and data annotation, specifically designed for deep learning dataset creation. The application provides intuitive tools for bounding box annotation, segmentation, and various image processing operations using ImGui, OpenGL, and OpenCV.

## Features

### Current Features (v1.0)
- **Image Viewer**: Full-featured image viewer with zoom, pan, and navigation
- **Interactive Controls**: Mouse wheel zoom, drag-to-pan, and button controls
- **Image Information**: Display image dimensions, channels, and zoom level
- **Pixel Information**: Real-time pixel coordinate and RGB value display
- **Test Image Generation**: Built-in test image creation for demonstration
- **Menu System**: Complete menu bar with File and View options

### Planned Features
- Bounding box annotation and editing
- Segmentation tools
- Template matching and feature matching
- AI-assisted annotation
- Batch processing capabilities
- Multi-format annotation export

## Quick Start

### Prerequisites
- C++20 compatible compiler (MSVC recommended)
- CMake 3.16 or higher
- OpenCV 4.x
- GLFW 3.x
- OpenGL 3.3+

### Building the Project

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd gui_imgui
   ```

2. **Install dependencies**:
   ```bash
   # Using vcpkg (recommended)
   vcpkg install opencv4 glfw imgui
   
   # Or using Conan
   conan install . --build=missing
   ```

3. **Build with CMake**:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

4. **Run the application**:
   ```bash
   ./gui_imgui  # Linux/Mac
   gui_imgui.exe  # Windows
   ```

### Quick Build (Windows)
```bash
build.bat
```

## Usage

### Basic Controls
- **Mouse Wheel**: Zoom in/out
- **Left Mouse + Drag**: Pan the image
- **Menu Bar**: Access File and View options
- **Zoom Buttons**: Use zoom in/out/reset buttons

### Menu Options
- **File > Create Test Image**: Generate a sample image for testing
- **File > Load Image...**: Load an image file (currently hardcoded)
- **View > Show Image Info**: Toggle image information display
- **View > Show Pixel Info**: Toggle pixel information display
- **View > Reset Zoom**: Reset zoom to 100%
- **View > Fit to Window**: Fit image to window size

## Project Structure

```
gui_imgui/
|-- include/                 # Header files
|   |-- core/               # Core system components
|   |-- graphics/           # Graphics backend abstraction
|   |-- image/              # Image handling and processing
|   |-- ui/                 # UI components and panels
|   |-- utils/              # Utility classes and helpers
|-- src/                    # Source files
|   |-- core/               # Core system implementation
|   |-- graphics/           # Graphics backend implementations
|   |-- image/              # Image processing implementation
|   |-- ui/                 # UI component implementations
|   |-- utils/              # Utility implementations
|-- docs/                   # Documentation
|   |-- plan.md             # Project plan and roadmap
|   |-- implement_rules.md  # Implementation guidelines
|   |-- finished.md         # Completed features tracking
|   |-- README.md           # This file
|-- examples/               # Example usage and demos
|-- main_temp.cpp           # Simple image viewer example
|-- main.cpp                # Full application (in development)
|-- CMakeLists.txt          # CMake build configuration
|-- build.bat               # Windows build script
```

## Development

### Getting Started with Development
1. Read `docs/plan.md` for the overall project plan
2. Review `docs/implement_rules.md` for coding standards
3. Check `docs/finished.md` for completed features
4. Start with `main_temp.cpp` to understand the basic structure

### Code Standards
- C++20 standard
- Comprehensive Korean comments for all complex code
- Consistent naming conventions (PascalCase for classes, camelCase for functions)
- RAII for resource management
- Exception safety guarantees

### Testing
- Manual testing with various image formats
- Performance testing for large images
- Memory leak detection
- Cross-platform compatibility testing

## Architecture

### Core Components
1. **Graphics Backend**: Abstracted OpenGL rendering system
2. **Image Management**: OpenCV-based image processing
3. **UI Framework**: ImGui-based user interface
4. **Data Management**: Centralized data handling

### Key Technologies
- **ImGui**: Immediate mode GUI library
- **OpenGL**: Hardware-accelerated graphics
- **OpenCV**: Computer vision and image processing
- **GLFW**: Window management and input handling
- **CMake**: Build system management

## Contributing

### Development Workflow
1. Create feature branch from `develop`
2. Implement changes following coding standards
3. Add comprehensive comments and documentation
4. Test thoroughly
5. Submit pull request with detailed description

### Code Review Guidelines
- All code must be reviewed before merging
- Ensure documentation is updated
- Validate performance impact
- Check for memory leaks and security issues

## Troubleshooting

### Common Issues

#### Compilation Errors
- **Missing Headers**: Ensure all dependencies are properly installed
- **OpenGL Constants**: Use `GL_CLAMP` instead of `GL_CLAMP_TO_EDGE` for compatibility
- **ImGui Flags**: Use only available ImGui window flags

#### Runtime Issues
- **Image Loading**: Check file paths and formats
- **OpenGL Context**: Ensure proper OpenGL version support
- **Memory Issues**: Monitor memory usage with large images

### Debugging Tips
- Use the detailed comments in `main_temp.cpp` as a reference
- Check console output for error messages
- Verify all dependencies are correctly linked
- Test with the built-in test image generation

## Performance

### Optimization Tips
- Use appropriate image sizes for your hardware
- Enable V-Sync for smooth rendering
- Monitor memory usage with large datasets
- Consider GPU acceleration for processing operations

### Benchmarks
- Image loading: <100ms for typical images
- Zoom/pan operations: 60 FPS target
- UI responsiveness: <16ms per frame
- Memory usage: Proportional to image size

## Roadmap

### Phase 1: Basic Tools (Current)
- [x] Image viewer with zoom/pan
- [x] Basic UI framework
- [ ] Simple annotation tools
- [ ] File I/O system

### Phase 2: Advanced Features
- [ ] Bounding box system
- [ ] Segmentation tools
- [ ] Template matching
- [ ] Batch processing

### Phase 3: AI Integration
- [ ] AI model integration
- [ ] Automated annotation
- [ ] Smart suggestions
- [ ] Learning capabilities

## License

This project is licensed under the MIT License. See LICENSE file for details.

## Support

For issues and questions:
1. Check the troubleshooting section
2. Review the documentation
3. Search existing issues
4. Create a new issue with detailed description

## Acknowledgments

- **ImGui**: For the excellent immediate mode GUI library
- **OpenCV**: For comprehensive computer vision tools
- **GLFW**: For cross-platform window management
- **OpenGL**: For hardware-accelerated graphics
