#include <windows.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

class HumanInputSim {
private:
    std::default_random_engine generator;
    
    // 조절 가능한 파라미터 (설정값)
    double speedModifier;   // 전체적인 동작 속도 (낮을수록 빠름, 0.5 => 2배빠름, 2 => 2배느림)
                            // 범위: 0.5 ~ 3.0 (권장: 0.8 ~ 1.5)
    double mistakeProb;     // 오타 및 실수 확률 (0.0 ~ 1.0)
                            // 범위: 0.0 ~ 1.0 (권장: 0.01 ~ 0.05, 100타중 1~5퍼센트오타발생)
    double randomness;      // 좌표 및 시간의 변동성 정도
                            // 범위: 0.1 ~ 0.5 (권장: 0.2 ~ 0.3)

    int screenWidth, screenHeight;

    // 가우시안 변동성을 적용한 지연 함수
    void HumanSleep(double mean) {
        std::normal_distribution<double> dist(mean, mean * randomness);
        double wait = (std::max)(mean * 0.3, dist(generator));
        Sleep((DWORD)wait);
    }

    // 베지어 곡선 포인트 계산
    POINT GetBezierPoint(POINT p0, POINT p1, POINT p2, double t) {
        POINT p;
        p.x = (long)((1 - t) * (1 - t) * p0.x + 2 * (1 - t) * t * p1.x + t * t * p2.x);
        p.y = (long)((1 - t) * (1 - t) * p0.y + 2 * (1 - t) * t * p1.y + t * t * p2.y);
        return p;
    }

public:
    HumanInputSim(double _speed = 1.0, double _mistake = 0.02, double _rand = 0.2) 
        : speedModifier(_speed), mistakeProb(_mistake), randomness(_rand) {
        screenWidth = GetSystemMetrics(SM_CXSCREEN);
        screenHeight = GetSystemMetrics(SM_CYSCREEN);
        std::random_device rd;
        generator.seed(rd());
    }

    // --- 마우스 제어 ---
    void MoveMouseHumanLike(int targetX, int targetY, int targetWidth = 1, int targetHeight = 1) {
        POINT start;
        GetCursorPos(&start);

        // 1. 최종 목적지 편차 적용 (버튼의 중앙이 아닌 랜덤한 지점)
        std::normal_distribution<double> devX((double)targetX + targetWidth / 2.0, targetWidth * randomness);
        std::normal_distribution<double> devY((double)targetY + targetHeight / 2.0, targetHeight * randomness);
        int finalX = (int)devX(generator);
        int finalY = (int)devY(generator);

        // 2. 곡률을 위한 제어점 생성 (직선 경로에서 의도적으로 벗어남)
        POINT p0 = start;
        POINT p2 = { finalX, finalY };
        int offsetX = (int)((p2.x - p0.x) * randomness);
        POINT p1 = { 
            (p0.x + p2.x) / 2 + (rand() % (offsetX + 1) - offsetX / 2),
            (p0.y + p2.y) / 2 + (rand() % 100 - 50) 
        };

        // 3. 부드러운 이동 실행
        int steps = (int)((20 + rand() % 10) * speedModifier);
        for (int i = 1; i <= steps; ++i) {
            double t = (double)i / steps;
            // 가속/감속 (Ease In-Out)
            double easedT = t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
            
            POINT next = GetBezierPoint(p0, p1, p2, easedT);
            
            INPUT input = { 0 };
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
            input.mi.dx = (next.x * 65535) / screenWidth;
            input.mi.dy = (next.y * 65535) / screenHeight;
            SendInput(1, &input, sizeof(INPUT));
            
            Sleep(5 + (rand() % 5));
        }
    }

    void Click(bool isPost = false, HWND hWnd = NULL) {
        HumanSleep(100 * speedModifier);
        if (isPost && hWnd) {
            PostMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, 0);
            HumanSleep(50 * speedModifier);
            PostMessage(hWnd, WM_LBUTTONUP, 0, 0);
        } else {
            INPUT input = { 0 };
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            SendInput(1, &input, sizeof(INPUT));
            HumanSleep(40);
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            SendInput(1, &input, sizeof(INPUT));
        }
    }

    // --- 키보드 제어 ---
    void TypeHumanLike(const std::wstring& text, HWND hWnd = NULL) {
        std::uniform_real_distribution<double> probDist(0.0, 1.0);

        for (wchar_t c : text) {
            // 오타 시뮬레이션
            if (probDist(generator) < mistakeProb) {
                PostMessage(hWnd ? hWnd : GetFocus(), WM_CHAR, (WPARAM)(c + 1), 0);
                HumanSleep(200);
                PostMessage(hWnd ? hWnd : GetFocus(), WM_KEYDOWN, VK_BACK, 0);
                HumanSleep(50);
            }

            if (hWnd) {
                PostMessage(hWnd, WM_CHAR, (WPARAM)c, 0);
            } else {
                INPUT input = { 0 };
                input.type = INPUT_KEYBOARD;
                input.ki.wScan = c;
                input.ki.dwFlags = KEYEVENTF_UNICODE;
                SendInput(1, &input, sizeof(INPUT));
                
                input.ki.dwFlags |= KEYEVENTF_KEYUP;
                HumanSleep(30);
                SendInput(1, &input, sizeof(INPUT));
            }
            HumanSleep(150 * speedModifier);
        }
    }
};


int main() {
    // 속도 1.0, 실수 확률 5%, 변동성 30%로 설정
    HumanInputSim human(1.0, 0.05, 0.3);

    // 1. (500, 500) 위치에 있는 가로 100, 세로 50 크기의 버튼으로 이동
    human.MoveMouseHumanLike(500, 500, 100, 50);
    
    // 2. 클릭
    human.Click();

    // 3. 실제 유저처럼 문자열 입력
    human.TypeHumanLike(L"Hello, this is a human-like input sim.");

    return 0;
}