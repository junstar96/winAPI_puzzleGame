#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT InitD3D(HWND hwnd);
void CleanD3D(void);
void RenderFrame(void);
void InitGraphics(void);
void InitPipeline(void);

IDXGISwapChain* swapchain;
ID3D11Device* dev;
ID3D11DeviceContext* devcon;
ID3D11RenderTargetView* backbuffer;
ID3D11VertexShader* pVS;
ID3D11PixelShader* pPS;
ID3D11InputLayout* pLayout;
ID3D11Buffer* pVBuffer;
ID3D11Buffer* pCBuffer;
ID3D11Buffer* pIBuffer; // 인덱스 버퍼 추가
XMMATRIX World, View, Projection;
float rotationSpeed = 0.5f;  // 회전 속도 조절 (기존 속도의 50%)

struct VERTEX {
    XMFLOAT3 Position;
    XMFLOAT4 Color;
};

struct CBUFFER {
    XMMATRIX Final;
};

VERTEX vertices[] = {
    {XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)},
    {XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f)},
    {XMFLOAT3(-0.5f, 0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)},
    {XMFLOAT3(0.5f, 0.5f, 0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f)}
};

int indices[] = { 0, 1, 2, 2, 1, 3 };

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(NULL,
        L"WindowClass",
        L"Direct3D 11 Tutorial",
        WS_OVERLAPPEDWINDOW,
        300, 300,
        800, 600,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hwnd, nCmdShow);

    if (FAILED(InitD3D(hwnd))) {
        return 0;
    }

    MSG msg;
    while (TRUE) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            RenderFrame();
        }
    }

    CleanD3D();

    return (WPARAM)msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        else if (wParam == 'F') {
            static bool fullscreen = false;
            fullscreen = !fullscreen;
            if (fullscreen) {
                SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
                SetWindowPos(hwnd, HWND_TOP, 0, 0, 800, 600, SWP_FRAMECHANGED);
            }
            else {
                SetWindowLong(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                SetWindowPos(hwnd, HWND_TOP, 300, 300, 800, 600, SWP_FRAMECHANGED);
            }
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HRESULT InitD3D(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC scd;
    ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 4;
    scd.Windowed = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D11CreateDeviceAndSwapChain(NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        NULL,
        NULL,
        NULL,
        D3D11_SDK_VERSION,
        &scd,
        &swapchain,
        &dev,
        NULL,
        &devcon);

    ID3D11Texture2D* pBackBuffer;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
    pBackBuffer->Release();
    devcon->OMSetRenderTargets(1, &backbuffer, NULL);

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 800;
    viewport.Height = 600;

    devcon->RSSetViewports(1, &viewport);

    InitPipeline();
    InitGraphics();

    return S_OK;
}

void InitPipeline()
{
    ID3DBlob* VS, * PS;
    D3DCompileFromFile(L"shader.fx", 0, 0, "VS", "vs_4_0", 0, 0, &VS, 0);
    D3DCompileFromFile(L"shader.fx", 0, 0, "PS", "ps_4_0", 0, 0, &PS, 0);

    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);
    devcon->VSSetShader(pVS, 0, 0);
    devcon->PSSetShader(pPS, 0, 0);

    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
    devcon->IASetInputLayout(pLayout);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    dev->CreateBuffer(&bd, NULL, &pVBuffer);

    D3D11_MAPPED_SUBRESOURCE ms;
    devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, vertices, sizeof(vertices));
    devcon->Unmap(pVBuffer, NULL);

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CBUFFER);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    dev->CreateBuffer(&bd, NULL, &pCBuffer);
}

void InitGraphics()
{
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    dev->CreateBuffer(&bd, NULL, &pVBuffer);

    D3D11_MAPPED_SUBRESOURCE ms;
    devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, vertices, sizeof(vertices));
    devcon->Unmap(pVBuffer, NULL);

    D3D11_BUFFER_DESC ibd;
    ZeroMemory(&ibd, sizeof(ibd));

    ibd.Usage = D3D11_USAGE_DYNAMIC;
    ibd.ByteWidth = sizeof(indices);
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    dev->CreateBuffer(&ibd, NULL, &pIBuffer); // 인덱스 버퍼 생성

    devcon->Map(pIBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, indices, sizeof(indices));
    devcon->Unmap(pIBuffer, NULL);

    World = XMMatrixIdentity();
    View = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -2.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, 800.0f / 600.0f, 0.01f, 100.0f);
}

void RenderFrame(void)
{
    static float time = 0.0f;
    time += 0.001f * rotationSpeed;

    World = XMMatrixRotationY(time);

    CBUFFER cBuffer;
    cBuffer.Final = XMMatrixTranspose(World * View * Projection);
    devcon->UpdateSubresource(pCBuffer, 0, NULL, &cBuffer, 0, 0);
    devcon->VSSetConstantBuffers(0, 1, &pCBuffer);

    const FLOAT clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f }; 
    devcon->ClearRenderTargetView(backbuffer, clearColor);

    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
    devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);
    devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    devcon->DrawIndexed(6, 0, 0);

    swapchain->Present(0, 0);
}


void CleanD3D(void)
{
    swapchain->SetFullscreenState(FALSE, NULL);
    pVS->Release();
    pPS->Release();
    pLayout->Release();
    pVBuffer->Release();
    pIBuffer->Release(); // 인덱스 버퍼 해제
    pCBuffer->Release();
    swapchain->Release();
    backbuffer->Release();
    dev->Release();
    devcon->Release();
}
