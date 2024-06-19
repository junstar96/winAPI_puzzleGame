#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <iostream>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")

// 전역 변수
ID2D1Factory* pD2DFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
IWICImagingFactory* pWICFactory = nullptr;
ID2D1Bitmap* pBitmap = nullptr;
IWICBitmap* pWICBitmap = nullptr;

// 윈도우 프로시저 선언
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 초기화 함수
HRESULT InitD2D(HWND hwnd);
void Cleanup();
void Render();
HRESULT LoadBitmapFromFile(LPCWSTR uri, ID2D1Bitmap** ppBitmap, IWICBitmap** ppWICBitmap);

// 진입점
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImagePalette", NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, L"Image Palette", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, wc.hInstance, NULL);

    if (SUCCEEDED(InitD2D(hwnd))) {
        ShowWindow(hwnd, nCmdShow);
        UpdateWindow(hwnd);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        Cleanup();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
    }
    return 0;
}

// 윈도우 프로시저
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT:
        Render();
        ValidateRect(hwnd, NULL);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        if (pWICBitmap) {
            IWICBitmapLock* pLock = nullptr;
            WICRect rect = { x, y, 1, 1 };
            HRESULT hr = pWICBitmap->Lock(&rect, WICBitmapLockRead, &pLock);
            if (SUCCEEDED(hr)) {
                UINT bufferSize = 0;
                BYTE* pData = nullptr;
                hr = pLock->GetDataPointer(&bufferSize, &pData);
                if (SUCCEEDED(hr)) {
                    UINT32 pixel = *reinterpret_cast<UINT32*>(pData);
                    BYTE r = (pixel & 0x00FF0000) >> 16;
                    BYTE g = (pixel & 0x0000FF00) >> 8;
                    BYTE b = (pixel & 0x000000FF);
                    std::wcout << L"Color at (" << x << L", " << y << L"): (" << r << L", " << g << L", " << b << L")" << std::endl;
                }
                pLock->Release();
            }
        }
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// Direct2D 초기화
HRESULT InitD2D(HWND hwnd) {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
    if (FAILED(hr)) return hr;

    RECT rc;
    GetClientRect(hwnd, &rc);

    hr = pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
        &pRenderTarget
    );
    if (FAILED(hr)) return hr;

    hr = CoInitialize(NULL);
    if (FAILED(hr)) return hr;

    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        (LPVOID*)&pWICFactory
    );
    if (FAILED(hr)) return hr;

    hr = LoadBitmapFromFile(L"example.jpg", &pBitmap, &pWICBitmap);
    if (FAILED(hr)) return hr;

    return S_OK;
}

// 정리 함수
void Cleanup() {
    if (pBitmap) pBitmap->Release();
    if (pRenderTarget) pRenderTarget->Release();
    if (pD2DFactory) pD2DFactory->Release();
    if (pWICFactory) pWICFactory->Release();
    if (pWICBitmap) pWICBitmap->Release();
    CoUninitialize();
}

// 렌더링 함수
void Render() {
    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

    if (pBitmap) {
        D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
        D2D1_SIZE_F bmpSize = pBitmap->GetSize();

        D2D1_RECT_F destRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);
        D2D1_RECT_F srcRect = D2D1::RectF(0, 0, bmpSize.width, bmpSize.height);

        pRenderTarget->DrawBitmap(pBitmap, destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srcRect);
    }

    pRenderTarget->EndDraw();
}

// 이미지 파일 로드 함수
HRESULT LoadBitmapFromFile(LPCWSTR uri, ID2D1Bitmap** ppBitmap, IWICBitmap** ppWICBitmap) {
    IWICBitmapDecoder* pDecoder = nullptr;
    IWICBitmapFrameDecode* pFrame = nullptr;
    IWICFormatConverter* pConverter = nullptr;

    HRESULT hr = pWICFactory->CreateDecoderFromFilename(uri, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &pDecoder);
    if (SUCCEEDED(hr)) {
        hr = pDecoder->GetFrame(0, &pFrame);
    }
    if (SUCCEEDED(hr)) {
        hr = pWICFactory->CreateFormatConverter(&pConverter);
    }
    if (SUCCEEDED(hr)) {
        hr = pConverter->Initialize(
            pFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
    }
    if (SUCCEEDED(hr)) {
        hr = pWICFactory->CreateBitmapFromSource(pConverter, WICBitmapCacheOnDemand, ppWICBitmap);
    }
    if (SUCCEEDED(hr)) {
        hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
    }

    if (pDecoder) pDecoder->Release();
    if (pFrame) pFrame->Release();
    if (pConverter) pConverter->Release();

    return hr;
}
