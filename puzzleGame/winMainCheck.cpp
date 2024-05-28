#include <windows.h>

// 윈도우 프로시저 함수 선언
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// WinMain 함수 - 프로그램의 시작점
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 윈도우 클래스 구조체 초기화
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASSW wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    // 윈도우 클래스 등록
    RegisterClassW(&wc);

    // 윈도우 생성
    HWND hwnd = CreateWindowExW(
        0,                              // 확장 스타일 옵션
        CLASS_NAME,                     // 윈도우 클래스 이름
        L"Sample Window",               // 윈도우 타이틀
        WS_OVERLAPPEDWINDOW,            // 윈도우 스타일
        CW_USEDEFAULT, CW_USEDEFAULT,   // 윈도우 초기 위치
        CW_USEDEFAULT, CW_USEDEFAULT,   // 윈도우 초기 크기
        NULL,                           // 부모 윈도우
        NULL,                           // 메뉴
        hInstance,                      // 인스턴스 핸들
        NULL                            // 추가 매개변수
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    // 윈도우 표시 및 업데이트
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 메시지 루프
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// 윈도우 프로시저 함수 정의
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 여기서 그리기 작업을 수행
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    return 0;

    // 다른 메시지 처리

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
