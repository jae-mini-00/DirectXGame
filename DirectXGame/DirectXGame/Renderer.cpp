#include "Renderer.h"

Renderer::Renderer() : device(nullptr), context(nullptr), swapChain(nullptr) {}
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

    return S_OK;
}

DXGI_SWAP_CHAIN_DESC Renderer::CreateswapChainDescInfo(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

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

FeatureLevelInfo Renderer::CreateFeatureLevelInfo() {
    FeatureLevelInfo Info = {};

    Info.levels[0] = D3D_FEATURE_LEVEL_11_0;
    Info.count = 1;

    return Info;
}