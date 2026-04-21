#include <windows.h>
#include <random>
#include <cmath>
#include <algorithm>

enum class InputMethod {
    SEND_INPUT,   // 시스템 전역 (실제 마우스 이동)
    POST_MESSAGE  // 특정 창 대상 (비활성 제어 가능)
};

class HumanInputSim {
private:
    std::default_random_engine generator;
    double m_speedModifier;   // 전체적인 동작 속도 (낮을수록 빠름, 0.5 => 2배빠름, 2 => 2배느림)
                            // 범위: 0.5 ~ 3.0 (권장: 0.8 ~ 1.5)
    double m_randomness;      // 좌표오차 및 시간의 변동성 정도
                            // 범위: 0.1 ~ 0.5 (권장: 0.2 ~ 0.3)
    double m_errorRange;      // 최종좌표 에러픽셀 수
                            //  (권장 : 2.0~3.0)

    void humanSleep(double mean) {
        std::normal_distribution<double> dist(mean, mean * m_randomness);
        double wait = (std::max)(mean * 0.3, dist(generator));
        Sleep((DWORD)wait);
    }

public:
    HumanInputSim(double _speed = 1.0, double _rand = 0.2, double errorRange= 2) 
        : m_speedModifier(_speed), m_randomness(_rand) {
        std::random_device rd;
        generator.seed(rd());
    }

    /**
     * @brief 마우스 이동 함수 (이동만 수행)
     * @param targetX 목적지 X
     * @param targetY 목적지 Y
     * @param method 입력 방식 (SEND_INPUT 또는 POST_MESSAGE)
     * @param hWnd POST_MESSAGE 사용 시 대상 창 핸들
     * @param errorRange 좌표 오차 범위
     */
    void moveMouse(int targetX, int targetY, InputMethod method, HWND hWnd = NULL, double errorRange = 0.0) {
        POINT start;
        if (method == InputMethod::SEND_INPUT) {
            GetCursorPos(&start);
        } else {
            // PostMessage 방식은 창 기준 좌표이므로 0,0에서 시작하거나 현재 논리적 위치 가정
            start = { 0, 0 }; 
        }
        if (errorRange == 0.0) errorRange = m_errorRange;
        // 1. 가우시안 오차 적용된 최종 좌표 계산
        std::normal_distribution<double> distX((double)targetX, errorRange);
        std::normal_distribution<double> distY((double)targetY, errorRange);
        int finalX = (int)distX(generator);
        int finalY = (int)distY(generator);

        // 2. 경로 생성을 위한 베지어 제어점
        POINT p0 = start;
        POINT p2 = { finalX, finalY };
        int dist = (int)sqrt(pow(p2.x - p0.x, 2) + pow(p2.y - p0.y, 2));
        int offset = (int)(dist * m_randomness);
        std::uniform_int_distribution<int> cDist(-offset, offset);
        POINT p1 = { (p0.x + p2.x) / 2 + cDist(generator), (p0.y + p2.y) / 2 + cDist(generator) };

        // 3. 단계별 이동
        int steps = (int)((15 + rand() % 10) * m_speedModifier);
        if (dist < 10) steps = 2; // 거리가 가까우면 단계 축소

        for (int i = 1; i <= steps; ++i) {
            double t = (double)i / steps;
            double easedT = t * t * (3 - 2 * t); // Smoothstep 가속
            
            int currX = (int)((1 - easedT) * (1 - easedT) * p0.x + 2 * (1 - easedT) * easedT * p1.x + easedT * easedT * p2.x);
            int currY = (int)((1 - easedT) * (1 - easedT) * p0.y + 2 * (1 - easedT) * easedT * p1.y + easedT * easedT * p2.y);

            if (method == InputMethod::SEND_INPUT) {
                INPUT input = { 0 };
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
                input.mi.dx = (currX * 65535) / GetSystemMetrics(SM_CXSCREEN);
                input.mi.dy = (currY * 65535) / GetSystemMetrics(SM_CYSCREEN);
                SendInput(1, &input, sizeof(INPUT));
            } else {
                // PostMessage는 실제 커서를 움직이지 않고 이동 메시지만 전송 (필요한 경우만)
                PostMessage(hWnd, WM_MOUSEMOVE, 0, MAKELPARAM(currX, currY));
            }
            Sleep(5 + (rand() % 5));
        }
    }

    /**
     * @brief 클릭 함수 (현재 위치에서 클릭만 수행)
     */
    void click(InputMethod method, HWND hWnd = NULL) {
        humanSleep(50 * m_speedModifier);

        if (method == InputMethod::SEND_INPUT) {
            INPUT input = { 0 };
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            SendInput(1, &input, sizeof(INPUT));
            humanSleep(40 + rand() % 30);
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(1, &input, sizeof(INPUT));
        } else {
            // PostMessage 방식은 현재 마우스 좌표를 얻어와서 그 지점을 클릭
            POINT p;
            GetCursorPos(&p); 
            if(hWnd) ScreenToClient(hWnd, &p); // 화면 좌표를 창 좌표로 변환

            PostMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(p.x, p.y));
            humanSleep(40 + rand() % 30);
            PostMessage(hWnd, WM_LBUTTONUP, 0, MAKELPARAM(p.x, p.y));
        }
    }
};


int main() {
    HumanInputSim human(1.0, 0.25); // 속도 1.0, 변동성 0.25
    HWND targetHWnd = FindWindowW(NULL, L"대상 프로그램 제목");

    // 상황 1: 실제 마우스를 움직여 전역 화면 클릭 (SendInput)
    human.moveMouse(800, 600, InputMethod::SEND_INPUT, NULL, 5.0);
    human.click(InputMethod::SEND_INPUT);

    // 상황 2: 특정 창에 메시지만 전송 (PostMessage)
    // 창 내부 좌표 (100, 100) 근처를 10픽셀 오차 범위 내에서 이동 후 클릭
    human.moveMouse(100, 100, InputMethod::POST_MESSAGE, targetHWnd, 10.0);
    human.click(InputMethod::POST_MESSAGE, targetHWnd);

    return 0;
}