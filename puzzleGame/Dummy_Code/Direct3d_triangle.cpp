#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

// ��ũ ���̺귯�� ����
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// ������ ���ν��� �Լ� ����
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Direct3D �ʱ�ȭ �� ���� �Լ� ����
HRESULT InitD3D(HWND hwnd);
void CleanD3D(void);
void RenderFrame(void);

// Direct3D ���� ����
IDXGISwapChain* swapchain;             // ���� ü��
ID3D11Device* dev;                     // ����̽�
ID3D11DeviceContext* devcon;           // ����̽� ���ؽ�Ʈ
ID3D11RenderTargetView* backbuffer;    // ���� Ÿ�� ��
ID3D11VertexShader* pVS;               // ���ؽ� ���̴�
ID3D11PixelShader* pPS;                // �ȼ� ���̴�
ID3D11InputLayout* pLayout;            // �Է� ���̾ƿ�
ID3D11Buffer* pVBuffer;                // ���ؽ� ����
ID3D11Buffer* pIBuffer;                // �ε��� ����
ID3D11Buffer* pCBuffer;                // ��� ����

// ��� ���� ����ü
struct CBUFFER {
    XMMATRIX Final;
};

// �����ü ���� ������ ����ü
struct VERTEX {
    XMFLOAT3 Position;
    XMFLOAT4 Color;
};

// �����ü ���� ������
VERTEX vertices[] = {
    { XMFLOAT3(0.0f,  1.0f,  0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
    { XMFLOAT3(0.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) }
};

// �����ü �ε��� ������
DWORD indices[] = {
    0, 1, 2,
    0, 2, 3,
    0, 3, 1,
    1, 2, 3
};

// WinMain �Լ� - ���α׷��� ������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // ������ Ŭ���� �ʱ�ȭ
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    // ������ ����
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

    // Direct3D �ʱ�ȭ
    if (FAILED(InitD3D(hwnd))) {
        return 0;
    }

    // �޽��� ����
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

    // Direct3D ����
    CleanD3D();

    return msg.wParam;
}

// ������ ���ν��� �Լ�
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Direct3D �ʱ�ȭ �Լ�
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

    // �� ���� ����
    ID3D11Texture2D* pBackBuffer;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
    pBackBuffer->Release();
    devcon->OMSetRenderTargets(1, &backbuffer, NULL);

    // ����Ʈ ����
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 800;
    viewport.Height = 600;

    devcon->RSSetViewports(1, &viewport);

    // ���̴� ������ �� ����
    ID3DBlob* VS, * PS;
    D3DCompileFromFile(L"shader.fx", 0, 0, "VShader", "vs_4_0", 0, 0, &VS, 0);
    D3DCompileFromFile(L"shader.fx", 0, 0, "PShader", "ps_4_0", 0, 0, &PS, 0);

    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);

    devcon->VSSetShader(pVS, 0, 0);
    devcon->PSSetShader(pPS, 0, 0);

    // �Է� ���̾ƿ� ����
    D3D11_INPUT_ELEMENT_DESC ied[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
    devcon->IASetInputLayout(pLayout);

    // ���ؽ� ���� ����
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

    // �ε��� ���� ����
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    dev->CreateBuffer(&bd, NULL, &pIBuffer);

    devcon->Map(pIBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, indices, sizeof(indices));
    devcon->Unmap(pIBuffer, NULL);

    // ��� ���� ����
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(CBUFFER);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    dev->CreateBuffer(&bd, NULL, &pCBuffer);

    return S_OK;
}

// Direct3D ���� �Լ�
void CleanD3D(void)
{
    swapchain->SetFullscreenState(FALSE, NULL);

    pLayout->Release();
    pVS->Release();
    pPS->Release();
    pVBuffer->Release();
    pIBuffer->Release();
    pCBuffer->Release();
    swapchain->Release();
    backbuffer->Release();
    dev->Release();
    devcon->Release();
}

// ���� ������ �Լ�
void RenderFrame(void)
{
    // ȭ�� �����
    float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    devcon->ClearRenderTargetView(backbuffer, color);

    // ȸ�� ��ȯ
    static float t = 0.0f;
    t += 0.0025f;

    XMMATRIX rot = XMMatrixRotationY(t);

    // ��� ���� ������Ʈ
    D3D11_MAPPED_SUBRESOURCE ms;
    devcon->Map(pCBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    ((CBUFFER*)ms.pData)->Final = XMMatrixTranspose(rot);
    devcon->Unmap(pCBuffer, NULL);

    devcon->VSSetConstantBuffers(0, 1, &pCBuffer);

    // ���� �� �ε��� ���� ����
    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);
    devcon->IASetIndexBuffer(pIBuffer, DXGI_FORMAT_R32_UINT, 0);
    devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // �׸���
    devcon->DrawIndexed(12, 0, 0);

    // ȭ�鿡 ���
    swapchain->Present(0, 0);
}
