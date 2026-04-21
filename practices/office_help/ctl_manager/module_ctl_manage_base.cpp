#include <windows.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>


// 마우스 클릭 함수
void mouseClick(int x, int y) {
    // 1. 커서 이동
    double screen_width = GetSystemMetrics(SM_CXSCREEN);
    double screen_height = GetSystemMetrics(SM_CYSCREEN);
    
    // SendInput은 0~65535 사이의 절대 좌표를 사용함
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
    input.mi.dx = (x * 65535 / screen_width);
    input.mi.dy = (y * 65535 / screen_height);
    SendInput(1, &input, sizeof(INPUT));
    
    Sleep(100); // 실제 유저처럼 약간의 지연시간

    // 2. 왼쪽 버튼 누름
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));
    
    Sleep(50); // 클릭 유지 시간

    // 3. 왼쪽 버튼 뗌
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

// 키보드 타이핑 함수
void typeString(const std::vector<WORD>& vkeys) {
    for (WORD vkey : vkeys) {
        INPUT input = { 0 };
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vkey;

        // Key Down
        SendInput(1, &input, sizeof(INPUT));
        Sleep(30 + rand() % 50); // 랜덤한 지연시간으로 감지 회피

        // Key Up
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
        
        Sleep(50 + rand() % 100);
    }
}

// 특정 핸들에 마우스 클릭 전송
void pMClick(HWND hWnd, int x, int y, float delay) {
    // 좌표를 LPARAM으로 변환 (하위 16비트 X, 상위 16비트 Y)
    LPARAM lParam = MAKELPARAM(x, y);

    PostMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, lParam);
    Sleep(delay);
    PostMessage(hWnd, WM_LBUTTONUP, 0, lParam);
}

// 특정 핸들에 키보드 입력 전송
void pKeyStroke(HWND hWnd, UINT vkey, float delay) {
    PostMessage(hWnd, WM_KEYDOWN, vkey, 0);
    Sleep(delay);
    PostMessage(hWnd, WM_KEYUP, vkey, 0);
}

struct Point {
    double x, y;
};

// 2차 베지어 곡선 계산: P0(시작), P1(제어점), P2(끝)
Point getBezierPoint(Point p0, Point p1, Point p2, double t) {
    double x = (1 - t) * (1 - t) * p0.x + 2 * (1 - t) * t * p1.x + t * t * p2.x;
    double y = (1 - t) * (1 - t) * p0.y + 2 * (1 - t) * t * p1.y + t * t * p2.y;
    return { x, y };
}

void curatedMMove(int targetX, int targetY) {
    POINT startPos;
    GetCursorPos(&startPos);

    Point p0 = { (double)startPos.x, (double)startPos.y };
    Point p2 = { (double)targetX, (double)targetY };
    
    // 제어점 p1을 시작점과 끝점 사이의 무작위 위치에 생성 (곡률 생성)
    Point p1 = {
        p0.x + (p2.x - p0.x) * ((rand() % 100) / 100.0),
        p0.y + (p2.y - p0.y) * ((rand() % 100) / 100.0) + (rand() % 100 - 50)
    };

    int steps = 20 + rand() % 15; // 이동 단계 수 (속도 조절)
    for (int i = 0; i <= steps; ++i) {
        double t = (double)i / steps;
        
        // 가속/감속 로직 (Sine 함수 이용)
        // 초반에 느리고 중간에 빠르며 끝에 다시 느려짐
        double easedT = -0.5 * (cos(3.14159 * t) - 1);
        
        Point next = getBezierPoint(p0, p1, p2, easedT);
        
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
        input.mi.dx = (long)(next.x * 65535 / GetSystemMetrics(SM_CXSCREEN));
        input.mi.dy = (long)(next.y * 65535 / GetSystemMetrics(SM_CYSCREEN));
        
        SendInput(1, &input, sizeof(INPUT));
        Sleep(10 + rand() % 10); // 미세 지연
    }
}


// 정규 분포를 따른 지연 시간 생성
void humanSleep(double mean, double stddev) {
    static std::default_random_engine generator;
    std::normal_distribution<double> distribution(mean, stddev);
    
    double waitTime = distribution(generator);
    // 음수가 나오지 않도록 최소값 방어 코드
    if (waitTime < mean * 0.5) waitTime = mean * 0.5;
    
    Sleep((DWORD)waitTime);
}

// 사용 예시: 평균 200ms, 표준편차 50ms로 대기
// HumanSleep(200, 50);

void humanType(HWND hWnd, const std::string& text) {
    for (char c : text) {
        // 2% 확률로 오타 발생
        if (rand() % 100 < 2) {
            PostMessage(hWnd, WM_CHAR, (WPARAM)(c + 1), 0); // 옆의 키 누름
            humanSleep(150, 30);
            PostMessage(hWnd, WM_KEYDOWN, VK_BACK, 0);      // 백스페이스
            humanSleep(50, 10);
            PostMessage(hWnd, WM_KEYUP, VK_BACK, 0);
            humanSleep(200, 50);
        }
        
        PostMessage(hWnd, WM_CHAR, (WPARAM)c, 0);
        
        // 키 간 입력 간격을 정규분포로 적용
        humanSleep(120, 40);
    }
}