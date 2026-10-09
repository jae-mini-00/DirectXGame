#include "Renderer.h"
#include "DxUtils.h"

Renderer::Renderer() : device(nullptr), context(nullptr), swapChain(nullptr), 
renderTargetView(nullptr), vertexShader(nullptr), pixelShader(nullptr),
inputLayout(nullptr), constantBuffer(nullptr), depthBuffer(nullptr), depthStencilView(nullptr) {}


Renderer::~Renderer() {
    if (depthStencilView)
        depthStencilView->Release();
    if (depthBuffer)
        depthBuffer->Release();
    if (constantBuffer)
        constantBuffer->Release();
    if (pixelShader)
        pixelShader->Release();
    if (vertexShader)
        vertexShader->Release();
    if (inputLayout)
        inputLayout->Release();
    if (renderTargetView)
        renderTargetView->Release();
    if (swapChain)
        swapChain->Release();
    if (context)
        context->Release();
    if (device)
        device->Release();
}

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

    RETURN_IF_FAILED(D3D11CreateDeviceAndSwapChain(pAdapter, DriverType, Software, Flags,
        FeatureInfo.levels, FeatureInfo.count, SDKVersion, pSwapChainDesc, 
        &swapChain, &device, pFeatureLevel, &context));

    RETURN_IF_FAILED(CreateRenderTarget());

    D3D11_VIEWPORT viewport = CreateViewport(hwnd);
    context->RSSetViewports(1, &viewport);


    RETURN_IF_FAILED(CreateConstantBuffer());

    RETURN_IF_FAILED(CreateVertexShader());
    RETURN_IF_FAILED(CreatePixelShader());

    
    CreateCameraMatrices(hwnd);

    RETURN_IF_FAILED(CreateDepthBuffer(hwnd));

    return S_OK;
}

void Renderer::CreateCameraMatrices(HWND hwnd) {
    DirectX::XMVECTOR eye =
        DirectX::XMVectorSet(0.0f, 0.0f, -3.0f, 0.0f);

    DirectX::XMVECTOR target =
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

    DirectX::XMVECTOR up =
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    view = DirectX::XMMatrixLookAtLH(eye, target, up);

    RECT rect;
    GetClientRect(hwnd, &rect);

    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);
    float aspect = width / height;

    projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(60.0f),
        aspect, 0.1f, 100.0f);
}

void Renderer::BindVertexStage() {
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(inputLayout);
    context->VSSetShader(vertexShader, nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
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
    RETURN_IF_FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));

    RETURN_IF_FAILED_AND_RELEASE(device->CreateRenderTargetView(backBuffer, nullptr, 
        &renderTargetView), backBuffer);
    backBuffer->Release();
    context->OMSetRenderTargets(1, &renderTargetView, nullptr);

    return S_OK;
}

FeatureLevelInfo Renderer::CreateFeatureLevelInfo() {
    FeatureLevelInfo Info = {};

    Info.levels[0] = D3D_FEATURE_LEVEL_11_0;
    Info.count = 1;

    return Info;
}

HRESULT Renderer::CreateConstantBuffer() {
    D3D11_BUFFER_DESC bufferDesc = {};

    bufferDesc.ByteWidth = sizeof(MatrixBuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    RETURN_IF_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &constantBuffer));

    return S_OK;
}

HRESULT Renderer::CreateInputLayout(ID3DBlob* shaderBlob) {
    D3D11_INPUT_ELEMENT_DESC layout[2] = {};

    layout[0].SemanticName = "POSITION";
    layout[0].SemanticIndex = 0;
    layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    layout[0].InputSlot = 0;
    layout[0].AlignedByteOffset = 0;
    layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout[0].InstanceDataStepRate = 0;
    layout[1].SemanticName = "COLOR";
    layout[1].SemanticIndex = 0;
    layout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    layout[1].InputSlot = 0;
    layout[1].AlignedByteOffset = 12;
    layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout[1].InstanceDataStepRate = 0;

    RETURN_IF_FAILED(device->CreateInputLayout(layout, 2, shaderBlob->GetBufferPointer(), 
        shaderBlob->GetBufferSize(), &inputLayout));


    return S_OK;
}

HRESULT Renderer::CreateVertexShader() {
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* shaderBlob = nullptr;
    RETURN_IF_FAILED_AND_RELEASE(D3DCompileFromFile(L"VertexShader.hlsl", nullptr,
        nullptr, "main", "vs_5_0", 0, 0, &shaderBlob, &errorBlob), errorBlob);
    if (errorBlob) {
        errorBlob->Release();
        errorBlob = nullptr;
    }

    RETURN_IF_FAILED_AND_RELEASE(device->CreateVertexShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &vertexShader), shaderBlob);

    RETURN_IF_FAILED_AND_RELEASE(CreateInputLayout(shaderBlob), shaderBlob);
    shaderBlob->Release();

    BindVertexStage();

    return S_OK;
}


HRESULT Renderer::CreatePixelShader() {
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* shaderBlob = nullptr;
    RETURN_IF_FAILED_AND_RELEASE(D3DCompileFromFile(L"PixelShader.hlsl", nullptr,
        nullptr, "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob), errorBlob);
    if (errorBlob) {
        errorBlob->Release();
        errorBlob = nullptr;
    }
    RETURN_IF_FAILED_AND_RELEASE(device->CreatePixelShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &pixelShader), shaderBlob);
    shaderBlob->Release();

    context->PSSetShader(pixelShader, nullptr, 0);

    return S_OK;
}

HRESULT Renderer::CreateDepthBuffer(HWND hwnd) {
    D3D11_TEXTURE2D_DESC depthDesc = {};

    RECT rect;
    GetClientRect(hwnd, &rect);
    depthDesc.Width = rect.right - rect.left;
    depthDesc.Height = rect.bottom - rect.top;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;

    RETURN_IF_FAILED(device->CreateTexture2D(&depthDesc, nullptr, &depthBuffer));
    
    RETURN_IF_FAILED(device->CreateDepthStencilView(depthBuffer, nullptr, &depthStencilView));

    context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    return S_OK;
}

DirectX::XMMATRIX Renderer::CreateTransform(const Transform& transform) {
    DirectX::XMMATRIX scale =
        DirectX::XMMatrixScaling(transform.scale.x,
            transform.scale.y, transform.scale.z);
    DirectX::XMMATRIX rotation =
        DirectX::XMMatrixRotationRollPitchYaw(
            DirectX::XMConvertToRadians(transform.rotation.x),
            DirectX::XMConvertToRadians(transform.rotation.y),
            DirectX::XMConvertToRadians(transform.rotation.z));
    DirectX::XMMATRIX translation =
        DirectX::XMMatrixTranslation(transform.position.x,
            transform.position.y, transform.position.z);

    return scale * rotation * translation;
}

void Renderer::UpdateMatrixBuffer(const DirectX::XMMATRIX& world) {
    MatrixBuffer matrixData = {};

    matrixData.world =
        DirectX::XMMatrixTranspose(world);

    matrixData.view =
        DirectX::XMMatrixTranspose(view);

    matrixData.projection =
        DirectX::XMMatrixTranspose(projection);

    context->UpdateSubresource(constantBuffer, 0, nullptr,
        &matrixData, 0, 0);
}

void Renderer::UpdateTransformBuffer(const Transform& transform) {
    DirectX::XMMATRIX world = CreateTransform(transform);

    UpdateMatrixBuffer(world);
}

void Renderer::Clear() {
    float rgbaColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    context->ClearRenderTargetView(renderTargetView, rgbaColor);

    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::BeginFrame()
{
    Clear();
}

void Renderer::Draw(const GameObject& object) {
    UpdateTransformBuffer(object.transform);

    UINT offset = 0;
    UINT stride = sizeof(Vertex);
    ID3D11Buffer* vertexBuffer =
        object.mesh.GetVertexBuffer();

    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(object.mesh.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
    context->DrawIndexed(object.mesh.GetIndexCount(), 0, 0);
}

void Renderer::EndFrame() {
    swapChain->Present(1, 0);
}

ID3D11Device* Renderer::GetDevice() const { return device; }