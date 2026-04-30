#pragma once

#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>

/**
 * @brief 다중 해상도 버퍼 관리 클래스
 * 
 * 이 클래스는 원본 이미지와 다양한 해상도의 프리뷰 이미지를
 * 효율적으로 관리하기 위한 버퍼 시스템을 제공합니다.
 * 
 * 주요 기능:
 * - 다중 해상도 버퍼 레벨 관리
 * - CPU/GPU 데이터 동기화
 * - 메모리 효율적인 데이터 관리
 * - 자동 해상도 조절
 */
class FrameBuffer {
public:
    /**
     * @brief 버퍼 레벨 정보
     * 각 레벨은 다른 해상도의 데이터를 저장
     */
    struct BufferLevel {
        cv::Mat cpuData;            // CPU 메모리 데이터 (OpenCV Mat)
        void* gpuTexture = nullptr; // GPU 텍스처 ID
        int width = 0;              // 버퍼 너비
        int height = 0;             // 버퍼 높이
        bool gpuDirty = true;       // GPU 업데이트 필요 여부
        bool cpuDirty = false;      // CPU 업데이트 필요 여부
        
        BufferLevel() = default;
        BufferLevel(int w, int h) : width(w), height(h) {}
    };

private:
    std::vector<BufferLevel> m_levels;  // 버퍼 레벨 목록 (0: 원본, 1: 프리뷰, 2: 썸네일...)
    int m_currentLevel = 0;              // 현재 활성 레벨
    std::string m_name;                 // 버퍼 이름 (디버깅용)
    bool m_isInitialized = false;       // 초기화 상태
    
public:
    /**
     * @brief 생성자
     * @param name 버퍼 이름
     */
    explicit FrameBuffer(const std::string& name = "");
    
    /**
     * @brief 소멸자 - 리소스 정리
     */
    ~FrameBuffer();
    
    /**
     * @brief 새로운 버퍼 레벨 추가
     * @param width 버퍼 너비
     * @param height 버퍼 높이
     * @return 추가된 레벨 인덱스
     */
    int addLevel(int width, int height);
    
    /**
     * @brief 원본 데이터 업데이트 (레벨 0)
     * @param data 새로운 이미지 데이터
     */
    void updateData(const cv::Mat& data);
    
    /**
     * @brief 특정 레벨의 데이터 업데이트
     * @param level 레벨 인덱스
     * @param data 새로운 데이터
     */
    void updateLevelData(int level, const cv::Mat& data);
    
    /**
     * @brief 특정 레벨의 GPU 텍스처 업데이트
     * @param level 레벨 인덱스
     * @param textureId GPU 텍스처 ID
     */
    void updateTexture(int level, void* textureId);
    
    /**
     * @brief GPU 텍스처 가져오기
     * @param level 레벨 인덱스 (기본값: 0)
     * @return GPU 텍스처 ID
     */
    void* getGPUTexture(int level = 0) const;
    
    /**
     * @brief 프리뷰 텍스처 가져오기 (자동으로 적절한 프리뷰 레벨 선택)
     * @param maxWidth 최대 프리뷰 너비
     * @param maxHeight 최대 프리뷰 높이
     * @return 프리뷰 텍스처 ID
     */
    void* getPreviewTexture(int maxWidth = 512, int maxHeight = 512) const;
    
    /**
     * @brief CPU 데이터 가져오기
     * @param level 레벨 인덱스 (기본값: 0)
     * @return OpenCV Mat 데이터
     */
    cv::Mat getCPUData(int level = 0) const;
    
    /**
     * @brief 버퍼 크기 정보 가져오기
     * @param level 레벨 인덱스
     * @param width 너비 출력 포인터
     * @param height 높이 출력 포인터
     */
    void getLevelSize(int level, int* width, int* height) const;
    
    /**
     * @brief 레벨 개수 가져오기
     * @return 레벨 개수
     */
    size_t getLevelCount() const { return m_levels.size(); }
    
    /**
     * @brief 현재 활성 레벨 설정
     * @param level 레벨 인덱스
     */
    void setCurrentLevel(int level);
    
    /**
     * @brief 현재 활성 레벨 가져오기
     * @return 현재 레벨 인덱스
     */
    int getCurrentLevel() const { return m_currentLevel; }
    
    /**
     * @brief 최적의 프리뷰 레벨 찾기
     * @param maxWidth 최대 너비
     * @param maxHeight 최대 높이
     * @return 최적의 레벨 인덱스
     */
    int findOptimalPreviewLevel(int maxWidth, int maxHeight) const;
    
    /**
     * @brief 모든 레벨의 GPU 데이터 정리
     */
    void cleanupGPUData();
    
    /**
     * @brief 모든 레벨의 데이터 정리
     */
    void cleanup();
    
    /**
     * @brief 초기화 상태 확인
     * @return 초기화 완료 여부
     */
    bool isInitialized() const { return m_isInitialized; }
    
    /**
     * @brief 유효한 레벨 인덱스인지 확인
     * @param level 레벨 인덱스
     * @return 유효 여부
     */
    bool isValidLevel(int level) const;
    
    /**
     * @brief 버퍼 정보 문자열 반환
     * @return 버퍼 정보
     */
    std::string getInfo() const;
};
