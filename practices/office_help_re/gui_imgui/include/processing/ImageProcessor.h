#pragma once

#include "IProcessingAlgorithm.h"
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>
#include <functional>

/**
 * @brief 스레드 안전한 큐 템플릿
 */
template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_condition;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(value));
        m_condition.notify_one();
    }
    
    bool tryPop(T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
    
    bool waitAndPop(T& value) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this] { return !m_queue.empty(); });
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
};

/**
 * @brief 이미지 프로세서 클래스
 * 
 * 이 클래스는 멀티스레드 환경에서 영상처리 알고리즘을
 * 실행하고 관리하기 위한 기능을 제공합니다.
 * 
 * 주요 기능:
 * - 멀티스레드 처리 지원
 * - 작업 큐 관리
 * - 알고리즘 등록 및 실행
 * - 비동기 결과 처리
 */
class ImageProcessor {
private:
    std::vector<std::unique_ptr<IProcessingAlgorithm>> m_algorithms;  // 등록된 알고리즘들
    std::vector<std::thread> m_workerThreads;                     // 작업자 스레드들
    ThreadSafeQueue<ProcessingTask> m_taskQueue;                     // 처리 작업 큐
    std::atomic<bool> m_shouldStop;                               // 종료 신호
    std::atomic<bool> m_isRunning;                                // 실행 상태
    mutable std::mutex m_algorithmsMutex;                           // 알고리즘 목록 뮤텍스
    size_t m_maxThreads;                                            // 최대 스레드 수
    std::atomic<size_t> m_activeTasks;                             // 활성 작업 수
    std::atomic<size_t> m_completedTasks;                           // 완료된 작업 수

public:
    /**
     * @brief 생성자
     * @param maxThreads 최대 스레드 수 (기본값: 하드웨어 동시성)
     */
    explicit ImageProcessor(size_t maxThreads = std::thread::hardware_concurrency());
    
    /**
     * @brief 소멸자
     */
    ~ImageProcessor();

    /**
     * @brief 프로세서 시작
     * @return 성공 여부
     */
    bool start();
    
    /**
     * @brief 프로세서 종료
     */
    void stop();
    
    /**
     * @brief 알고리즘 등록
     * @param algorithm 등록할 알고리즘
     * @return 성공 여부
     */
    bool registerAlgorithm(std::unique_ptr<IProcessingAlgorithm> algorithm);
    
    /**
     * @brief 알고리즘 제거
     * @param algorithmName 제거할 알고리즘 이름
     * @return 성공 여부
     */
    bool unregisterAlgorithm(const std::string& algorithmName);
    
    /**
     * @brief 처리 작업 제출 (비동기)
     * @param task 처리할 작업
     * @return 처리 결과 Future
     */
    std::future<ProcessingResult> submitTask(const ProcessingTask& task);
    
    /**
     * @brief 즉시 처리 작업 실행 (동기)
     * @param task 처리할 작업
     * @return 처리 결과
     */
    ProcessingResult executeTask(const ProcessingTask& task);
    
    /**
     * @brief 알고리즘으로 이미지 처리
     * @param algorithmName 알고리즘 이름
     * @param image 입력 이미지
     * @param parameters 알고리즘 파라미터
     * @return 처리 결과
     */
    ProcessingResult processImage(const std::string& algorithmName, 
                            const cv::Mat& image,
                            const std::map<std::string, double>& parameters = {});
    
    /**
     * @brief 비동기 이미지 처리
     * @param algorithmName 알고리즘 이름
     * @param image 입력 이미지
     * @param callback 완료 콜백
     * @param parameters 알고리즘 파라미터
     */
    void processImageAsync(const std::string& algorithmName,
                       const cv::Mat& image,
                       std::function<void(const ProcessingResult&)> callback,
                       const std::map<std::string, double>& parameters = {});
    
    /**
     * @brief 등록된 알고리즘 목록 가져오기
     * @return 알고리즘 이름 목록
     */
    std::vector<std::string> getAvailableAlgorithms() const;
    
    /**
     * @brief 특정 알고리즘 가져오기
     * @param algorithmName 알고리즘 이름
     * @return 알고리즘 포인터 (없으면 nullptr)
     */
    IProcessingAlgorithm* getAlgorithm(const std::string& algorithmName) const;
    
    /**
     * @brief 실행 상태 확인
     * @return 실행 중 여부
     */
    bool isRunning() const { return m_isRunning.load(); }
    
    /**
     * @brief 활성 작업 수 가져오기
     * @return 활성 작업 수
     */
    size_t getActiveTaskCount() const { return m_activeTasks.load(); }
    
    /**
     * @brief 완료된 작업 수 가져오기
     * @return 완료된 작업 수
     */
    size_t getCompletedTaskCount() const { return m_completedTasks.load(); }
    
    /**
     * @brief 대기 중인 작업 수 가져오기
     * @return 대기 작업 수
     */
    size_t getPendingTaskCount() const { return m_taskQueue.size(); }
    
    /**
     * @brief 스레드 수 가져오기
     * @return 현재 스레드 수
     */
    size_t getThreadCount() const { return m_workerThreads.size(); }
    
    /**
     * @brief 모든 알고리즘 초기화
     * @return 성공 여부
     */
    bool initializeAlgorithms();
    
    /**
     * @brief 모든 알고리즘 종료
     */
    void cleanupAlgorithms();

private:
    /**
     * @brief 작업자 스레드 함수
     */
    void workerThread();
    
    /**
     * @brief 작업 실행
     * @param task 실행할 작업
     * @return 처리 결과
     */
    ProcessingResult executeProcessingTask(const ProcessingTask& task);
    
    /**
     * @brief 알고리즘 찾기
     * @param algorithmName 알고리즘 이름
     * @return 알고리즘 포인터
     */
    IProcessingAlgorithm* findAlgorithm(const std::string& algorithmName) const;
};
