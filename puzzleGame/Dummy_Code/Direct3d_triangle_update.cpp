#include <windows.h>
#include <d3d11.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <string>
#pragma comment (lib, "d3d11.lib")

// 윈도우 프로시저 선언
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// DirectX 전역 변수
IDXGISwapChain* swapChain;
ID3D11Device* device;
ID3D11DeviceContext* deviceContext;
ID3D11RenderTargetView* renderTargetView;

void InitD3D(HWND hwnd);
void CleanD3D();
void RenderFrame();

void readTextFile(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Error opening text file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(infile, line)) {
        std::cout << line << std::endl;
    }
    infile.close();
}

void writeTextFile(const std::string& filename, const std::string& content) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error opening text file: " << filename << std::endl;
        return;
    }

    outfile << content;
    outfile.close();
}

void readBMP(const std::string& filename) {
    std::ifstream bmpFile(filename, std::ios::binary);
    if (!bmpFile.is_open()) {
        std::cerr << "Error opening BMP file: " << filename << std::endl;
        return;
    }

#pragma pack(push, 1)
    struct BMPHeader {
        uint16_t bfType;
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffBits;
    };

    struct BMPInfoHeader {
        uint32_t biSize;
        int32_t biWidth;
        int32_t biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        int32_t biXPelsPerMeter;
        int32_t biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
    };
#pragma pack(pop)

    BMPHeader header;
    bmpFile.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.bfType != 0x4D42) {
        std::cerr << "Not a BMP file: " << filename << std::endl;
        bmpFile.close();
        return;
    }

    BMPInfoHeader infoHeader;
    bmpFile.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    std::cout << "Width: " << infoHeader.biWidth << std::endl;
    std::cout << "Height: " << infoHeader.biHeight << std::endl;
    std::cout << "Bit Count: " << infoHeader.biBitCount << std::endl;

    bmpFile.seekg(header.bfOffBits, std::ios::beg);

    int rowSize = ((infoHeader.biBitCount * infoHeader.biWidth + 31) / 32) * 4;
    int dataSize = rowSize * abs(infoHeader.biHeight);
    std::vector<uint8_t> data(dataSize);

    bmpFile.read(reinterpret_cast<char*>(data.data()), dataSize);

    bmpFile.close();

    // 여기서 이미지 데이터를 처리할 수 있습니다
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 윈도우 클래스 설정
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = L"WindowClass";
    RegisterClassEx(&wc);

    // 윈도우 생성
    HWND hwnd = CreateWindowEx(0, L"WindowClass", L"DirectX Example", WS_OVERLAPPEDWINDOW, 300, 300, 800, 600, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    // DirectX 초기화
    InitD3D(hwnd);

    // 메시지 루프
    MSG msg;
    while (TRUE) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT) {
            break;
        }

        RenderFrame();
    }

    // DirectX 정리
    CleanD3D();

    return msg.wParam;
}

// 윈도우 프로시저
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// DirectX 초기화 함수
void InitD3D(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = 800;
    scd.BufferDesc.Height = 600;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 4;
    scd.Windowed = TRUE;
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &scd, &swapChain, &device, NULL, &deviceContext);

    ID3D11Texture2D* pBackBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    device->CreateRenderTargetView(pBackBuffer, NULL, &renderTargetView);
    pBackBuffer->Release();

    deviceContext->OMSetRenderTargets(1, &renderTargetView, NULL);

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 800;
    viewport.Height = 600;
    deviceContext->RSSetViewports(1, &viewport);
}

// DirectX 정리 함수
void CleanD3D() {
    swapChain->SetFullscreenState(FALSE, NULL);
    renderTargetView->Release();
    swapChain->Release();
    device->Release();
    deviceContext->Release();
}

// 프레임 렌더링 함수
void RenderFrame() {
    // 화면을 파란색으로 클리어
    float color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
    deviceContext->ClearRenderTargetView(renderTargetView, color);

    // 여기서 렌더링 코드를 추가할 수 있습니다

    swapChain->Present(0, 0);
}
