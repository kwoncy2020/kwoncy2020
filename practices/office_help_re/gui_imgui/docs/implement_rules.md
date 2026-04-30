# Implementation Rules and Guidelines

## Code Style and Standards

### General Principles
- **Clarity over cleverness**: Write code that is easy to understand
- **Consistency**: Maintain consistent coding style throughout the project
- **Documentation**: Document all public interfaces and complex logic
- **Modularity**: Keep components focused and loosely coupled
- **Testability**: Write code that can be easily tested

### Naming Conventions
- **Classes**: PascalCase (e.g., `ImageProcessor`, `BoundingBoxManager`)
- **Functions/Methods**: camelCase (e.g., `processImage()`, `loadFromFile()`)
- **Variables**: camelCase (e.g., `currentImage`, `zoomLevel`)
- **Constants**: UPPER_SNAKE_CASE (e.g., `MAX_IMAGE_SIZE`, `DEFAULT_ZOOM`)
- **File Names**: PascalCase for classes, snake_case for utilities
- **Private Members**: Prefix with `m_` (e.g., `m_currentImage`, `m_isInitialized`)

### File Organization
```
Header Files (.h, .hpp):
- Include guards
- Forward declarations
- Class/interface declarations
- Public interface first, private members last

Source Files (.cpp):
- Include corresponding header first
- Implementation details
- Private helper functions
- Order: public methods, protected methods, private methods
```

## Documentation Standards

### Header File Documentation
```cpp
/**
 * @file ClassName.h
 * @brief Brief description of the file's purpose
 * 
 * Detailed description of the file's role in the system,
 * including key responsibilities and usage examples.
 */

#pragma once

#include <necessary>
#include <headers>

/**
 * @brief Brief class description
 * 
 * Detailed class description including:
 * - Main responsibilities
 * - Key features
 * - Usage examples
 * - Thread safety considerations
 * - Performance characteristics
 */
class ClassName {
public:
    /**
     * @brief Brief method description
     * @param paramName Parameter description
     * @return Return value description
     * @throws ExceptionType When and why this exception is thrown
     * 
     * Detailed method description including:
     * - Algorithm description (if complex)
     * - Performance considerations
     * - Side effects
     * - Usage examples
     */
    ReturnType methodName(ParamType paramName);
};
```

### Implementation Documentation
```cpp
/**
 * @brief Implementation details for complex methods
 * 
 * For complex algorithms or non-obvious implementations,
 * provide detailed explanation of the approach, trade-offs,
 * and any relevant references or algorithms used.
 */
ReturnType ClassName::complexMethod(ParamType param) {
    // Step 1: Input validation
    if (!isValid(param)) {
        throw std::invalid_argument("Invalid parameter");
    }
    
    // Step 2: Core algorithm implementation
    // [Detailed comments explaining algorithm steps]
    
    return result;
}
```

### Comment Guidelines
- **What and Why**: Explain what the code does and why it's done that way
- **Complex Logic**: Document non-obvious algorithms or business logic
- **Assumptions**: State any assumptions made in the code
- **Trade-offs**: Document any trade-offs or design decisions
- **TODO/FIXME**: Use standardized tags with clear descriptions

## Architecture Guidelines

### Module Design
- **Single Responsibility**: Each module has one clear purpose
- **Interface Segregation**: Keep interfaces focused and minimal
- **Dependency Inversion**: Depend on abstractions, not concretions
- **Open/Closed Principle**: Open for extension, closed for modification

### Class Design
- **Encapsulation**: Keep implementation details private
- **Composition over Inheritance**: Prefer composition when possible
- **RAII**: Use RAII for resource management
- **Exception Safety**: Provide strong exception safety guarantees

### Error Handling
```cpp
// Use exceptions for exceptional circumstances
class ImageProcessingException : public std::runtime_error {
public:
    explicit ImageProcessingException(const std::string& message)
        : std::runtime_error("Image Processing Error: " + message) {}
};

// Return error codes for expected failures
enum class ProcessingResult {
    Success,
    InvalidInput,
    InsufficientMemory,
    ProcessingError
};
```

## Performance Guidelines

### Memory Management
- **Smart Pointers**: Use `std::unique_ptr` and `std::shared_ptr` appropriately
- **Avoid Copies**: Use move semantics and references to avoid unnecessary copies
- **Memory Pools**: Consider memory pools for frequently allocated objects
- **Resource Cleanup**: Ensure all resources are properly cleaned up

### Image Processing Performance
- **Lazy Evaluation**: Defer expensive operations until necessary
- **Caching**: Cache results of expensive computations
- **GPU Acceleration**: Use OpenGL shaders for image operations when beneficial
- **Batch Processing**: Process multiple images together when possible

### UI Performance
- **60 FPS Target**: Maintain 60 FPS for smooth UI interaction
- **Async Operations**: Use background threads for long-running operations
- **Progress Indicators**: Show progress for operations taking more than 100ms
- **Responsive Design**: Keep UI responsive during processing

## Testing Guidelines

### Unit Testing
```cpp
// Test structure using Google Test framework
class ImageProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test data
    }
    
    void TearDown() override {
        // Cleanup
    }
    
    ImageProcessor processor;
};

TEST_F(ImageProcessorTest, BasicProcessing) {
    // Test basic functionality
    cv::Mat input = createTestImage();
    cv::Mat result = processor.process(input);
    
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result.size(), input.size());
}
```

### Integration Testing
- **Component Interaction**: Test how components work together
- **End-to-End Workflows**: Test complete user workflows
- **Performance Testing**: Validate performance requirements
- **Memory Testing**: Check for memory leaks and excessive usage

### Code Coverage
- **Target**: Aim for >80% code coverage
- **Critical Paths**: 100% coverage for critical code paths
- **Edge Cases**: Test all edge cases and error conditions
- **Regression Tests**: Add tests for all bug fixes

## Build and Deployment

### CMake Guidelines
```cmake
# Use modern CMake practices
cmake_minimum_required(VERSION 3.16)
project(GuiImGui VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use target-based approach
add_executable(gui_imgui main.cpp)
target_link_libraries(gui_imgui PRIVATE imgui opencv glfw)

# Include directories
target_include_directories(gui_imgui PRIVATE ${CMAKE_SOURCE_DIR}/include)
```

### Dependency Management
- **Version Pinning**: Pin dependency versions for reproducible builds
- **Package Managers**: Use Conan or vcpkg for C++ dependencies
- **Git Submodules**: Use git submodules for version-controlled dependencies
- **Documentation**: Document all dependencies and their versions

## Security Guidelines

### Input Validation
- **File Validation**: Validate all input files for format and size
- **Path Traversal**: Prevent path traversal attacks
- **Resource Limits**: Enforce limits on resource usage
- **Error Messages**: Avoid revealing sensitive information in error messages

### Code Security
- **Buffer Overflows**: Use safe string operations and bounds checking
- **Memory Safety**: Use smart pointers and avoid raw pointers
- **Type Safety**: Use strong typing and avoid void pointers
- **Static Analysis**: Use static analysis tools to detect security issues

## Git Workflow

### Commit Messages
```
type(scope): brief description

Detailed description of changes:
- What was changed
- Why it was changed
- Any breaking changes
- References to issues

Examples:
feat(ui): add bounding box drawing tool
fix(image): resolve memory leak in image loading
docs(readme): update installation instructions
```

### Branch Strategy
- **main**: Stable, production-ready code
- **develop**: Integration branch for new features
- **feature/***: Feature-specific branches
- **bugfix/***: Bug fix branches
- **release/***: Release preparation branches

### Code Review Process
- **Peer Review**: All code must be reviewed before merging
- **Automated Checks**: Run tests and static analysis
- **Documentation**: Ensure documentation is updated
- **Performance**: Validate performance impact

## Debugging and Logging

### Logging Strategy
```cpp
// Use structured logging
#include <spdlog/spdlog.h>

void processImage(const cv::Mat& image) {
    spdlog::info("Starting image processing: {}x{}", image.cols, image.rows);
    
    try {
        // Processing logic
        spdlog::debug("Processing step 1 completed");
        // More processing
        spdlog::info("Image processing completed successfully");
    } catch (const std::exception& e) {
        spdlog::error("Image processing failed: {}", e.what());
        throw;
    }
}
```

### Debugging Guidelines
- **Asserts**: Use assertions for development-time checks
- **Debug Builds**: Enable additional checks in debug builds
- **Memory Debugging**: Use tools like Valgrind or AddressSanitizer
- **Performance Profiling**: Profile performance bottlenecks

## External Dependencies

### Library Usage Guidelines
- **Minimal Dependencies**: Keep external dependencies to a minimum
- **Stable Versions**: Use stable, well-tested library versions
- **Documentation**: Document why each dependency is needed
- **Alternatives**: Consider alternatives for heavy dependencies

### ImGui Best Practices
- **Immediate Mode**: Understand immediate mode GUI paradigm
- **ID Management**: Use proper widget IDs to avoid conflicts
- **Performance**: Minimize widget count and update frequency
- **Styling**: Use consistent styling throughout the application

### OpenCV Integration
- **Mat Management**: Understand OpenCV Mat memory management
- **Format Conversion**: Handle BGR/RGB conversions properly
- **Threading**: Be aware of OpenCV threading considerations
- **GPU Acceleration**: Use OpenCV GPU modules when beneficial
