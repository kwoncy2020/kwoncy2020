# GUI Build Plan

## Project Overview
GUI application using wxWidgets and OpenCV with C++20 modules system. The application provides image processing capabilities with a main interface, image viewer, and zoom functionality.

## 최종 앱의 형태
앱은 이미지를 로드하고, 로드된 이미지에서 특정 영역을 선택할 수 있는 기능을 제공합니다. (여러개의 바운딩박스)
선택된 영역은 bounding box로 표시되며, 선택된 영역을 기반으로 추가적인 확대용 윈도우창을 생성해서 더 정교한 학습용 데이터를 얻을 수 있도록 합니다. (기본 윈도우창에서도 동일하게 데이터를 얻을 수 있게만듬).
바운딩박스로 찾아진 영역은 학습용 데이터 또는 매칭알고리즘으로 다른이미지에서 찾는데 사용할 수 있도록 저장할 수 있습니다.
학습용 데이터를 얻게 된 이후에 여러가지 이미지프로세싱 (템플릿 매칭, 특징점 매칭, 이진화, 외곽선 검출) 과 AI적용 또한 가능해야함.

## 기술 스택
- **GUI**: wxWidgets
- **이미지 처리**: OpenCV
- **언어**: C++20 modules
- **패턴**: MVC, Repository, Dependency Injection


## 현재 상태 및 제약사항
1. 현재 ImageData클래스에서 이미지 format을 제대로 추적하고 있지 않습니다. 채널이 3개인경우 rgb인지 bgr인지 구분하지 않고 있습니다. 이를 확실히 구분해야하며 이 값을 제대로 알려면 opencv에서 파일을 어떻게 열었는지 알아야합니다. 또한 opencv말고도 다른 라이브러리로도 이미지를 처리해야할 경우가 생길 수 있기에 처음 ImageData를 생성할때 format을 함께 전달해야합니다.2. 현재는 opencv라이브러리 하나를 사용하는데, 다른 라이브러리를 사용할 수 도 있으니 Controller에서 처리하는게 좋아보입니다.



## 주요 목표: Controller를 통한 관심사 분리
1. Controller를 추가하여 GUI부분에서 관심사를 분리할 예정입니다.
2. 파일로드로 얻게된 이미지데이터에 대한 정보는 ImageData클래스가 관리합니다.
3. Controller는 Repository를 통해 ImageData를 관리합니다.
4. ImageProcessor는 모든 이미지 처리 및 렌더링을 담당하고 Controller가 이를 조율합니다.

## 최종 아키텍처
```
GUI → ScrolledImageWindow → ImageController → ImageDataRepository → ImageData (데이터만)
                                              ↓
                                      ImageProcessor (모든 처리/렌더링)
                                              ↓
                                      GUI에 최종 결과 전달
```

## 각 계층의 책임
- **GUI (ScrolledImageWindow)**: UI 로직 및 사용자 상호작용, Controller를 통해 화면에 그릴 이미지 데이터 얻기
- **Controller**: 비즈니스 로직 및 데이터 흐름 제어, ImageProcessor를 통한 처리 파이프라인 조율
- **Repository**: ImageData 저장/조회/삭제 관리
- **ImageData**: 순수 데이터 모델 (원본 이미지, 바운딩박스, 메타데이터)
- **ImageProcessor**: 모든 이미지 처리 (렌더링, 이진화, 외곽선 검출, 템플릿 매칭 등)






## 상세 구현 계획

### Phase 1: 기본 아키텍처 구축 ✅
1. **Controller 클래스 설계 및 구현** ✅
2. **Repository와의 연동** ✅
3. **ImageData 메타데이터 기능 확장** ✅
4. **기본 Controller 기능 테스트** ✅

### Phase 2: 아키텍처 재설계 및 GUI 연동 ✅
1. **ImageProcessor가 모든 처리 담당하도록 재설계** ✅
2. **ImageData에서 렌더링 로직 제거** (진행 중)
3. **ScrolledImageWindow에 Controller 주입 구현** ✅
4. **Repository 직접 참조 제거** ✅

### Phase 3: 이미지 처리 기능 확장
1. **ImageProcessor에 고급 처리 기능 추가**
   - 이진화, 외곽선 검출, 템플릿 매칭
   - 특징점 매칭, AI 적용 준비
2. **Controller에서 처리 파이프라인 구현**
   - 데이터 수집 → 전처리 → 특징추출 → 매칭
3. **성능 최적화**
   - ScrolledImageWindow에서 이미지 캐싱
   - 변경 시에만 렌더링 업데이트

## 진행 상황

### ✅ 완료된 작업
1. **Phase 1: 기본 아키텍처 구축** ✅
2. **Phase 2: 아키텍처 재설계 및 GUI 연동** ✅
   - Controller 기본 구조 설계 및 구현
   - Repository와의 연동
   - ImageData 메타데이터 기능 확장
   - ScrolledImageWindow에 Controller 주입
   - Repository 직접 참조 제거
   - 아키텍처 재설계 (ImageProcessor가 모든 처리 담당)

### 🔄 현재 진행 중
3. **Phase 3: 이미지 처리 기능 확장**
   - [진행 중] ImageData에서 렌더링 로직 제거 및 ImageProcessor로 이전
   - [대기] ImageProcessor에 고급 처리 기능 추가
   - [대기] Controller에서 처리 파이프라인 구현
   - [대기] 성능 최적화 (캐싱)

## 우선순위 (최종 업데이트)
1. [완료] Controller 기본 구조 구현
2. [완료] Repository와의 연동
3. [완료] 기존 GUI와의 최소 연동
4. [완료] 빌드 및 기본 기능 테스트
5. [완료] 아키텍처 재설계
6. [진행 중] ImageData 렌더링 로직 제거
7. [높음] ImageProcessor 고급 처리 기능 추가
8. [높음] Controller 처리 파이프라인 구현
9. [중간] 성능 최적화 (캐싱)
10. [낮음] 스크롤윈도우 리팩토링 (추후)