#include <windows.h>

// ������ ���ν��� �Լ� ����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// WinMain �Լ� - ���α׷��� ������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // ������ Ŭ���� ����ü �ʱ�ȭ
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASSW wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    // ������ Ŭ���� ���
    RegisterClassW(&wc);

    // ������ ����
    HWND hwnd = CreateWindowExW(
        0,                              // Ȯ�� ��Ÿ�� �ɼ�
        CLASS_NAME,                     // ������ Ŭ���� �̸�
        L"Sample Window",               // ������ Ÿ��Ʋ
        WS_OVERLAPPEDWINDOW,            // ������ ��Ÿ��
        CW_USEDEFAULT, CW_USEDEFAULT,   // ������ �ʱ� ��ġ
        CW_USEDEFAULT, CW_USEDEFAULT,   // ������ �ʱ� ũ��
        NULL,                           // �θ� ������
        NULL,                           // �޴�
        hInstance,                      // �ν��Ͻ� �ڵ�
        NULL                            // �߰� �Ű�����
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    // ������ ǥ�� �� ������Ʈ
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // �޽��� ����
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// ������ ���ν��� �Լ� ����
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

        // ���⼭ �׸��� �۾��� ����
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    return 0;

    // �ٸ� �޽��� ó��

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
