# Backend-Neutral Image Processing Application

백엔드 중립적이고 모듈화된 이미지 처리 애플리케이션입니다. 엔터프라이즈급 영상처리 솔루션을 목표로 설계되었습니다.

## 🎯 핵심 특징

- **완전한 백엔드 중립성**: OpenGL, Vulkan, DirectX, Metal 지원
- **강력한 영상처리**: 멀티스레드 처리, 알고리즘 파이프라인
- **다양한 데이터 소스**: 이미지, 비디오, 카메라, 화면 캡처
- **모듈화된 UI**: 분리된 패널 시스템 (메뉴, 컨트롤, 상태)
- **중앙 데이터 관리**: 통합된 상태 관리 및 히스토리
- **확장성**: 플러그인 아키텍처, 쉬운 확장

## 🏗️ 아키텍처 개요

### 계층 구조
```
┌─────────────────────────────────────────┐
│                UI Layer                   │
│  MenuBar │ ControlPanel │ StatusBar      │
├─────────────────────────────────────────┤
│              Utils Layer                  │
│           DataManager                     │
├─────────────────────────────────────────┤
│            Processing Layer               │
│  ImageProcessor │ ProcessingPipeline     │
│  Algorithms: BBox, Segmentation, Match    │
├─────────────────────────────────────────┤
│             Sources Layer                 │
│  ImageFile │ Video │ Camera │ Window     │
├─────────────────────────────────────────┤
│              Graphics Layer               │
│  IGraphicsBackend │ OpenGLBackend       │
├─────────────────────────────────────────┤
│                Core Layer                  │
│  IDataSource │ FrameBuffer │ Image       │
└─────────────────────────────────────────┘
```

## 📁 완전한 파일 구조

```
gui_imgui/
├── include/
│   ├── core/                    # 핵심 인터페이스
│   │   ├── IGraphicsBackend.h
│   │   ├── IDataSource.h
│   │   └── FrameBuffer.h
│   ├── graphics/                # 그래픽 백엔드
│   │   ├── IGraphicsBackend.h
│   │   ├── OpenGLBackend.h
│   │   └── BackendFactory.h
│   ├── image/                   # 이미지 처리
│   │   ├── IImageLoader.h
│   │   ├── Image.h
│   │   └── ImageViewer.h
│   ├── processing/             # 영상처리
│   │   ├── IProcessingAlgorithm.h
│   │   ├── ImageProcessor.h
│   │   ├── ProcessingPipeline.h
│   │   └── algorithms/
│   │       ├── BoundingBoxDetection.h
│   │       ├── SegmentationAlgorithm.h
│   │       └── MatchingAlgorithm.h
│   ├── sources/                 # 데이터 소스
│   │   ├── ImageFileSource.h
│   │   ├── VideoFileSource.h
│   │   ├── CameraSource.h
│   │   └── WindowCaptureSource.h
│   ├── ui/                      # UI 패널
│   │   ├── MenuBar.h
│   │   ├── ControlPanel.h
│   │   └── StatusBar.h
│   ├── utils/                   # 유틸리티
│   │   └── DataManager.h
│   └── backends/                # 이미지 로더 백엔드
│       └── OpenGLImageLoader.h
├── src/
│   ├── core/
│   ├── graphics/
│   ├── image/
│   ├── processing/
│   │   └── algorithms/
│   ├── sources/
│   ├── ui/
│   ├── utils/
│   └── backends/
├── docs/
│   ├── ARCHITECTURE_SUMMARY.md
│   ├── IMPLEMENTATION_GUIDE.md
│   └── COMPONENT_REFERENCE.md
└── examples/
    └── basic_usage.cpp
```

## 🚀 빌드 방법

### 의존성
- **CMake 3.30+**
- **C++20** 컴파일러
- **vcpkg** 패키지 관리자
- **ImGui** 즉시 모드 GUI
- **GLFW** 창 관리
- **OpenCV** 컴퓨터 비전
- **OpenGL** 그래픽 렌더링

### 빌드 절차

1. **vcpkg 설치**
   ```bash
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

2. **의존성 설치**
   ```bash
   vcpkg install imgui glfw3 opencv:x64-windows-static
   ```

3. **CMake 빌드**
   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build . --config Release
   ```

## 💻 사용 방법

### 기본 사용법
```cpp
#include "utils/DataManager.h"
#include "ui/MenuBar.h"
#include "ui/ControlPanel.h"
#include "ui/StatusBar.h"

int main() {
    // 데이터 관리자 초기화
    DataManager dataManager;
    dataManager.initialize();
    
    // UI 패널 생성
    MenuBar menuBar;
    ControlPanel controlPanel;
    StatusBar statusBar;
    
    // 콜백 설정
    setupCallbacks(menuBar, controlPanel, statusBar, dataManager);
    
    // 메인 루프
    while (running) {
        menuBar.render();
        controlPanel.render();
        renderImageViewer(dataManager);
        statusBar.render();
        updateUIState(menuBar, controlPanel, statusBar, dataManager);
    }
    
    return 0;
}
```

### 이미지 처리
```cpp
// 이미지 로딩
dataManager.loadImage("input.jpg");

// 단일 알고리즘 처리
dataManager.processImage("BoundingBoxDetection");

// 파이프라인 처리
auto pipeline = dataManager.getProcessingPipeline();
pipeline->addAlgorithm("BoundingBoxDetection");
pipeline->addAlgorithm("Segmentation");
dataManager.processWithPipeline();

// 결과 저장
dataManager.saveProcessedImage("result.jpg");
```

### 다양한 데이터 소스
```cpp
// 비디오 파일
dataManager.loadVideo("video.mp4");

// 카메라
dataManager.openCamera(0);

// 윈도우 캡처
dataManager.openWindowCapture("Calculator");

// 데스크톱 캡처
dataManager.openDesktopCapture();
```

## 🔧 확장 방법

### 새로운 알고리즘 추가
```cpp
class CustomAlgorithm : public IProcessingAlgorithm {
public:
    std::string getName() const override { return "Custom Algorithm"; }
    ProcessingResult process(const cv::Mat& input) override {
        // 알고리즘 구현
    }
};

// 등록
auto algorithm = std::make_shared<CustomAlgorithm>();
imageProcessor->registerAlgorithm("CustomAlgorithm", algorithm);
```

### 새로운 데이터 소스 추가
```cpp
class NetworkSource : public IDataSource {
    // 네트워크 소스 구현
};
```

### 새로운 UI 패널 추가
```cpp
class CustomPanel {
public:
    void render();
    void setCallbacks(/* 콜백 파라미터 */);
};
```

## 📊 성능 특징

- **멀티스레딩**: ImageProcessor의 스레드 풀 기반 병렬 처리
- **비동기 처리**: UI 응답성 유지를 위한 비동기 작업 실행
- **메모리 최적화**: 스마트 포인터와 효율적인 버퍼 관리
- **GPU 가속**: OpenGL 기반 이미지 표시 및 처리

## 📚 문서

- **[아키텍처 요약](docs/ARCHITECTURE_SUMMARY.md)** - 전체 아키텍처 개요
- **[구현 가이드](docs/IMPLEMENTATION_GUIDE.md)** - 상세 구현 및 사용법
- **[컴포넌트 레퍼런스](docs/COMPONENT_REFERENCE.md)** - API 레퍼런스

## 🎯 구현된 기능

### ✅ 완료됨
- [x] 백엔드 중립적 그래픽 시스템
- [x] 멀티스레드 영상처리 엔진
- [x] 3개의 완전한 알고리즘 (BBox, Segmentation, Matching)
- [x] 4종류 데이터 소스 (이미지, 비디오, 카메라, 캡처)
- [x] 모듈화된 UI 패널 시스템
- [x] 중앙 데이터 관리자
- [x] 처리 히스토리 (실행 취소/다시 실행)

### 🔄 향후 계획
- [ ] Vulkan 백엔드 구현
- [ ] 머신러닝 모델 통합
- [ ] 플러그인 시스템
- [ ] 클라우드 처리 지원
- [ ] 웹 인터페이스

## 🏆 기술 스택

- **C++20**: 최신 C++ 기능 활용
- **OpenCV**: 컴퓨터 비전 및 이미지 처리
- **ImGui**: 즉시 모드 GUI 프레임워크
- **GLFW**: 크로스플랫폼 창 관리
- **CMake**: 빌드 시스템
- **vcpkg**: 패키지 관리

## 📄 라이선스

이 프로젝트는 교육 및 연구 목적으로 개발되었습니다.

---

**업데이트**: 완전한 엔터프라이즈급 이미지 처리 아키텍처가 구현되었습니다. 모든 핵심 컴포넌트가 완성되었으며, 확장 가능한 설계를 통해 미래 기술 변화에 쉽게 적응할 수 있습니다.

