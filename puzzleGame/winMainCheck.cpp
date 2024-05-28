#include <windows.h>

// ��ư ID ����
#define BUTTON_BLUE 1
#define BUTTON_RED 2
#define BUTTON_YELLOW 3

// ������ ���ν��� �Լ� ����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ColorWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HINSTANCE g_hInstance;

// WinMain �Լ� - ���α׷��� ������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    g_hInstance = hInstance;
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASSW wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Sample Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// ���� ������ ���� �Լ�
void CreateColorWindow(COLORREF color)
{
    const wchar_t COLOR_CLASS_NAME[] = L"Color Window Class";

    WNDCLASSW wc = { };

    wc.lpfnWndProc = ColorWindowProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = COLOR_CLASS_NAME;

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        COLOR_CLASS_NAME,
        L"Color Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 300,
        NULL,
        NULL,
        g_hInstance,
        (LPVOID)color // ���� ���� ������ �߰� �Ű������� ����
    );

    if (hwnd != NULL)
    {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }
}

// ���� ������ ���ν��� �Լ�
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        // ��ư ����
        CreateWindowW(L"BUTTON", L"Blue",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            150, 50, 200, 50,
            hwnd, (HMENU)BUTTON_BLUE, g_hInstance, NULL);

        CreateWindowW(L"BUTTON", L"Red",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            150, 150, 200, 50,
            hwnd, (HMENU)BUTTON_RED, g_hInstance, NULL);

        CreateWindowW(L"BUTTON", L"Yellow",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            150, 250, 200, 50,
            hwnd, (HMENU)BUTTON_YELLOW, g_hInstance, NULL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case BUTTON_BLUE:
            CreateColorWindow(RGB(0, 0, 255));
            break;

        case BUTTON_RED:
            CreateColorWindow(RGB(255, 0, 0));
            break;

        case BUTTON_YELLOW:
            CreateColorWindow(RGB(255, 255, 0));
            break;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// ���� ������ ���ν��� �Լ�
LRESULT CALLBACK ColorWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static COLORREF color;

    switch (uMsg)
    {
    case WM_CREATE:
        color = (COLORREF)((CREATESTRUCT*)lParam)->lpCreateParams;
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HBRUSH brush = CreateSolidBrush(color);

        FillRect(hdc, &ps.rcPaint, brush);
        DeleteObject(brush);
        EndPaint(hwnd, &ps);
    }
    return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
