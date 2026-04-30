#pragma once

#include <string>
#include <memory>
#include <opencv2/opencv.hpp>

/**
 * @brief 백엔드 중립적 이미지 로더 인터페이스
 * 
 * 이 인터페이스는 다양한 그래픽 백엔드(OpenGL, Vulkan, DirectX 등)에서
 * 이미지를 로드하고 관리하기 위한 공통 API를 제공합니다.
 * 
 * 주요 기능:
 * - 정적 이미지 파일 로딩
 * - 실시간 비디오 프레임 처리
 * - 멀티 해상도 텍스처 지원
 * - 메모리 효율적인 텍스처 재사용
 */
class IImageLoader {
public:
    virtual ~IImageLoader() = default;

    /**
     * @brief 파일에서 이미지를 로드하여 텍스처 생성
     * @param filepath 이미지 파일 경로
     * @return 성공 시 텍스처 ID, 실패 시 nullptr
     */
    virtual void* loadImageFromFile(const std::string& filepath) = 0;

    /**
     * @brief OpenCV Mat 데이터에서 텍스처 생성
     * @param mat OpenCV Mat 이미지 데이터
     * @param width 출력될 텍스처 너비 포인터
     * @param height 출력될 텍스처 높이 포인터
     * @return 성공 시 텍스처 ID, 실패 시 nullptr
     */
    virtual void* createTextureFromMat(const cv::Mat& mat, int* width, int* height) = 0;

    /**
     * @brief 기존 텍스처에 새로운 프레임 데이터 업데이트 (비디오용)
     * @param textureId 기존 텍스처 ID
     * @param frame 새로운 프레임 데이터
     * @return 성공 여부
     */
    virtual bool updateTextureFromFrame(void* textureId, const cv::Mat& frame) = 0;

    /**
     * @brief 스트리밍용 텍스처 생성 (동영상/카메라용)
     * @return 텍스처 ID
     */
    virtual void* createStreamingTexture() = 0;

    /**
     * @brief 프리뷰용 저해상도 텍스처 생성
     * @param frame 원본 프레임 데이터
     * @param previewWidth 프리뷰 너비
     * @param previewHeight 프리뷰 높이
     * @return 프리뷰 텍스처 ID
     */
    virtual void* createPreviewTexture(const cv::Mat& frame, int previewWidth, int previewHeight) = 0;

    /**
     * @brief 텍스처 크기 정보 가져오기
     * @param textureId 텍스처 ID
     * @param width 너비 출력 포인터
     * @param height 높이 출력 포인터
     */
    virtual void getTextureSize(void* textureId, int* width, int* height) = 0;

    /**
     * @brief 텍스처 삭제 및 리소스 해제
     * @param textureId 삭제할 텍스처 ID
     */
    virtual void destroyTexture(void* textureId) = 0;

    /**
     * @brief 모든 텍스처 리소스 정리
     */
    virtual void cleanup() = 0;

    /**
     * @brief 백엔드 초기화 상태 확인
     * @return 초기화 완료 여부
     */
    virtual bool isInitialized() const = 0;
};
