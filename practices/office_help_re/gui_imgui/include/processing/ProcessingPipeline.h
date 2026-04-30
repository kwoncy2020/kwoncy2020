#pragma once

#include "IProcessingAlgorithm.h"
#include <vector>
#include <memory>
#include <functional>
#include <future>

/**
 * @brief 처리 파이프라인 클래스
 * 
 * 이 클래스는 여러 영상처리 알고리즘을 체인으로 연결하여
 * 순차적으로 실행하는 기능을 제공합니다.
 * 
 * 주요 기능:
 * - 알고리즘 체이닝
 * - 순차적 처리 실행
 * - 중간 결과 저장
 * - 비동기 파이프라인 실행
 * - 동적 알고리즘 추가/제거
 */
class ProcessingPipeline {
private:
    std::vector<std::unique_ptr<IProcessingAlgorithm>> m_pipeline;  // 알고리즘 파이프라인
    std::vector<ProcessingResult> m_intermediateResults;      // 중간 결과들
    bool m_saveIntermediateResults;                           // 중간 결과 저장 여부
    bool m_enabled;                                          // 파이프라인 활성화 여부
    mutable std::mutex m_pipelineMutex;                         // 파이프라인 뮤텍스

public:
    /**
     * @brief 생성자
     * @param saveIntermediateResults 중간 결과 저장 여부
     */
    explicit ProcessingPipeline(bool saveIntermediateResults = false);
    
    /**
     * @brief 소멸자
     */
    ~ProcessingPipeline();

    /**
     * @brief 알고리즘을 파이프라인에 추가
     * @param algorithm 추가할 알고리즘
     * @return 추가된 위치 (실패 시 -1)
     */
    int addAlgorithm(std::unique_ptr<IProcessingAlgorithm> algorithm);
    
    /**
     * @brief 파이프라인에서 알고리즘 제거
     * @param index 제거할 알고리즘 인덱스
     * @return 성공 여부
     */
    bool removeAlgorithm(int index);
    
    /**
     * @brief 파이프라인에서 알고리즘 제거 (이름으로)
     * @param algorithmName 제거할 알고리즘 이름
     * @return 성공 여부
     */
    bool removeAlgorithm(const std::string& algorithmName);
    
    /**
     * @brief 파이프라인에서 알고리즘 찾기
     * @param algorithmName 찾을 알고리즘 이름
     * @return 알고리즘 포인터 (없으면 nullptr)
     */
    IProcessingAlgorithm* getAlgorithm(const std::string& algorithmName) const;
    
    /**
     * @brief 파이프라인 실행 (동기)
     * @param input 입력 이미지
     * @return 최종 처리 결과
     */
    ProcessingResult execute(const cv::Mat& input);
    
    /**
     * @brief 파이프라인 실행 (비동기)
     * @param input 입력 이미지
     * @param callback 완료 콜백
     * @return 처리 결과 Future
     */
    std::future<ProcessingResult> executeAsync(const cv::Mat& input, 
                                           std::function<void(const ProcessingResult&)> callback = nullptr);
    
    /**
     * @brief 특정 위치부터 파이프라인 실행
     * @param input 입력 이미지
     * @param startIndex 시작 알고리즘 인덱스
     * @return 처리 결과
     */
    ProcessingResult executeFrom(const cv::Mat& input, int startIndex);
    
    /**
     * @brief 특정 범위의 알고리즘만 실행
     * @param input 입력 이미지
     * @param startIndex 시작 인덱스
     * @param endIndex 끝 인덱스
     * @return 처리 결과
     */
    ProcessingResult executeRange(const cv::Mat& input, int startIndex, int endIndex);
    
    /**
     * @brief 파이프라인 초기화
     * @return 성공 여부
     */
    bool initialize();
    
    /**
     * @brief 파이프라인 종료
     */
    void cleanup();
    
    /**
     * @brief 파이프라인 비우기
     */
    void clear();
    
    /**
     * @brief 파이프라인 활성화 여부 설정
     * @param enabled 활성화 여부
     */
    void setEnabled(bool enabled);
    
    /**
     * @brief 중간 결과 저장 여부 설정
     * @param save 저장 여부
     */
    void setSaveIntermediateResults(bool save);
    
    /**
     * @brief 파이프라인 크기 가져오기
     * @return 알고리즘 개수
     */
    size_t size() const;
    
    /**
     * @brief 파이프라인이 비었는지 확인
     * @return 비었는지 여부
     */
    bool empty() const;
    
    /**
     * @brief 알고리즘 목록 가져오기
     * @return 알고리즘 이름 목록
     */
    std::vector<std::string> getAlgorithmNames() const;
    
    /**
     * @brief 중간 결과들 가져오기
     * @return 중간 결과 목록
     */
    std::vector<ProcessingResult> getIntermediateResults() const;
    
    /**
     * @brief 특정 알고리즘의 결과 가져오기
     * @param algorithmName 알고리즘 이름
     * @return 처리 결과 (없으면 실패한 결과)
     */
    ProcessingResult getAlgorithmResult(const std::string& algorithmName) const;
    
    /**
     * @brief 파이프라인 정보 가져오기
     * @return 파이프라인 정보 문자열
     */
    std::string getPipelineInfo() const;
    
    /**
     * @brief 파이프라인 복제
     * @return 복제된 파이프라인
     */
    std::unique_ptr<ProcessingPipeline> clone() const;
    
    /**
     * @brief 알고리즘 순서 바꾸기
     * @param fromIndex 원본 인덱스
     * @param toIndex 목표 인덱스
     * @return 성공 여부
     */
    bool swapAlgorithms(int fromIndex, int toIndex);
    
    /**
     * @brief 알고리즘을 파이프라인의 특정 위치에 삽입
     * @param algorithm 삽입할 알고리즘
     * @param index 삽입 위치
     * @return 성공 여부
     */
    bool insertAlgorithm(std::unique_ptr<IProcessingAlgorithm> algorithm, int index);

private:
    /**
     * @brief 알고리즘 인덱스 찾기
     * @param algorithmName 알고리즘 이름
     * @return 알고리즘 인덱스 (없으면 -1)
     */
    int findAlgorithmIndex(const std::string& algorithmName) const;
    
    /**
     * @brief 단일 알고리즘 실행
     * @param algorithm 실행할 알고리즘
     * @param input 입력 이미지
     * @return 처리 결과
     */
    ProcessingResult executeAlgorithm(IProcessingAlgorithm* algorithm, const cv::Mat& input);
    
    /**
     * @brief 중간 결과 저장
     * @param result 저장할 결과
     */
    void saveIntermediateResult(const ProcessingResult& result);
    
    /**
     * @brief 중간 결과들 비우기
     */
    void clearIntermediateResults();
};
