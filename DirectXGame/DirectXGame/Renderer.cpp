#include "Renderer.h"

Renderer::Renderer() : device(nullptr), context(nullptr), swapChain(nullptr), renderTargetView(nullptr) {}
Renderer::~Renderer() {}

HRESULT Renderer::Initialize(HWND hwnd) {
    IDXGIAdapter* pAdapter = nullptr;
    D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
    HMODULE Software = nullptr;
    UINT Flags = 0;

    FeatureLevelInfo FeatureInfo = CreateFeatureLevelInfo();
    D3D_FEATURE_LEVEL* pFeatureLevel = &FeatureInfo.selectedLevel;
    UINT SDKVersion = D3D11_SDK_VERSION;
    DXGI_SWAP_CHAIN_DESC swapChainDesc = CreateswapChainDescInfo(hwnd);
    const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc = &swapChainDesc;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags,
        FeatureInfo.levels, FeatureInfo.count, SDKVersion, pSwapChainDesc, 
        &swapChain, &device, pFeatureLevel, &context);
    if (FAILED(hr))
        return hr;


    hr = CreateRenderTarget();
    if (FAILED(hr))
        return hr;

    D3D11_VIEWPORT viewport = CreateViewport(hwnd);
    context->RSSetViewports(1, &viewport);
    return S_OK;
}

D3D11_VIEWPORT Renderer::CreateViewport(HWND hwnd) {
    D3D11_VIEWPORT viewport = {};
    RECT rect;

    GetClientRect(hwnd, &rect);
    viewport.Width = static_cast<float>(rect.right - rect.left);
    viewport.Height = static_cast<float>(rect.bottom - rect.top);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    return viewport;
}

DXGI_SWAP_CHAIN_DESC Renderer::CreateswapChainDescInfo(HWND hwnd) {
    RECT rect;
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

    GetClientRect(hwnd, &rect);
    swapChainDesc.BufferDesc.Width = rect.right - rect.left;
    swapChainDesc.BufferDesc.Height = rect.bottom - rect.top;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    return swapChainDesc;
}

HRESULT Renderer::CreateRenderTarget() {
    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(hr))
        return hr;

    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    backBuffer->Release();
    if (FAILED(hr))
        return hr;

    context->OMSetRenderTargets(1, &renderTargetView, nullptr);

    return S_OK;
}

FeatureLevelInfo Renderer::CreateFeatureLevelInfo() {
    FeatureLevelInfo Info = {};

    Info.levels[0] = D3D_FEATURE_LEVEL_11_0;
    Info.count = 1;

    return Info;
}

void Renderer::Render() {
    float rgbaColor[4] = {0.0f, 0.2f, 0.4f, 1.0f};
    context->ClearRenderTargetView(renderTargetView, rgbaColor);
    swapChain->Present(1, 0);
}