#include "Core/Renderer.h"
#include "Scene/GameObject.h"
#include "Core/DxUtils.h"
#include "Core/Camera.h"
#include "Core/Logger/Logger.h"
#include "Graphics/Material.h"
#include "Core/Paths.h"
#include "Graphics/Animator.h"

#include <sstream>
#include <iomanip>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")

ID3D11Device* Renderer::GetDevice() const { return device; }
ID3D11DeviceContext* Renderer::GetContext() const { return context; }

Renderer::Renderer() : device(nullptr), context(nullptr), swapChain(nullptr), 
renderTargetView(nullptr), vertexShader(nullptr), pixelShader(nullptr),
inputLayout(nullptr), matrixBuffer(nullptr), depthBuffer(nullptr),
depthStencilView(nullptr), materialBuffer(nullptr), lightBuffer(nullptr),
boneMatrixBuffer(nullptr) {}


Renderer::~Renderer()
{
    if (boneMatrixBuffer)
        boneMatrixBuffer->Release();
    if (lightBuffer)
        lightBuffer->Release();
    if (materialBuffer)
        materialBuffer->Release();
    if (depthStencilView)
        depthStencilView->Release();
    if (depthBuffer)
        depthBuffer->Release();
    if (matrixBuffer)
        matrixBuffer->Release();
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

HRESULT Renderer::Initialize(HWND hwnd)
{
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


    RETURN_IF_FAILED(CreateMatrixBuffer());
    RETURN_IF_FAILED(CreateMaterialBuffer());
    RETURN_IF_FAILED(CreateLightBuffer());
    RETURN_IF_FAILED(CreateBoneMatrixBuffer());

    RETURN_IF_FAILED(CreateVertexShader());
    RETURN_IF_FAILED(CreatePixelShader());

    RETURN_IF_FAILED(CreateDepthBuffer(hwnd));

    return S_OK;
}


void Renderer::BindVertexStage()
{
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(inputLayout);
    context->VSSetShader(vertexShader, nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &matrixBuffer);
    context->VSSetConstantBuffers(1, 1, &boneMatrixBuffer);
}

void Renderer::BindPixelStage()
{
    context->PSSetShader(pixelShader, nullptr, 0);
    context->PSSetConstantBuffers(0, 1, &materialBuffer);
    context->PSSetConstantBuffers(1, 1, &lightBuffer);
}

// TODO: WM_SIZE 발생 시 RenderTarget, DepthBuffer,
//       Viewport, Camera Projection을 새 창 크기에 맞게 갱신
D3D11_VIEWPORT Renderer::CreateViewport(HWND hwnd)
{
    D3D11_VIEWPORT viewport = {};
    RECT rect;

    GetClientRect(hwnd, &rect);
    viewport.Width = static_cast<float>(rect.right - rect.left);
    viewport.Height = static_cast<float>(rect.bottom - rect.top);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    return viewport;
}

DXGI_SWAP_CHAIN_DESC Renderer::CreateswapChainDescInfo(HWND hwnd)
{
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

HRESULT Renderer::CreateRenderTarget()
{
    ID3D11Texture2D* backBuffer = nullptr;
    RETURN_IF_FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));

    RETURN_IF_FAILED_AND_RELEASE(device->CreateRenderTargetView(backBuffer, nullptr, 
        &renderTargetView), backBuffer);
    backBuffer->Release();
    context->OMSetRenderTargets(1, &renderTargetView, nullptr);

    return S_OK;
}

FeatureLevelInfo Renderer::CreateFeatureLevelInfo()
{
    FeatureLevelInfo Info = {};

    Info.levels[0] = D3D_FEATURE_LEVEL_11_0;
    Info.count = 1;

    return Info;
}

HRESULT Renderer::CreateMatrixBuffer()
{
    D3D11_BUFFER_DESC bufferDesc = {};

    bufferDesc.ByteWidth = sizeof(MatrixBuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    RETURN_IF_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &matrixBuffer));

    return S_OK;
}

HRESULT Renderer::CreateMaterialBuffer()
{
    D3D11_BUFFER_DESC bufferDesc = {};

    bufferDesc.ByteWidth = sizeof(MaterialBuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    RETURN_IF_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &materialBuffer));

    return S_OK;
}

HRESULT Renderer::CreateLightBuffer()
{
    D3D11_BUFFER_DESC bufferDesc = {};

    bufferDesc.ByteWidth = sizeof(LightBuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    RETURN_IF_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &lightBuffer));

    return S_OK;
}

HRESULT Renderer::CreateInputLayout(ID3DBlob* shaderBlob)
{
    D3D11_INPUT_ELEMENT_DESC layout[5] = {};

    layout[0].SemanticName = "POSITION";
    layout[0].SemanticIndex = 0;
    layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    layout[0].InputSlot = 0;
    layout[0].AlignedByteOffset = offsetof(Vertex, x);
    layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout[0].InstanceDataStepRate = 0;

    layout[1].SemanticName = "TEXCOORD";
    layout[1].SemanticIndex = 0;
    layout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    layout[1].InputSlot = 0;
    layout[1].AlignedByteOffset = offsetof(Vertex, u);
    layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout[1].InstanceDataStepRate = 0;

    layout[2].SemanticName = "NORMAL";
    layout[2].SemanticIndex = 0;
    layout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    layout[2].InputSlot = 0;
    layout[2].AlignedByteOffset = offsetof(Vertex, nx);
    layout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout[2].InstanceDataStepRate = 0;

    layout[3].SemanticName = "BLENDINDICES";
    layout[3].SemanticIndex = 0;
    layout[3].Format = DXGI_FORMAT_R32G32B32A32_SINT;
    layout[3].InputSlot = 0;
    layout[3].AlignedByteOffset = offsetof(Vertex, boneIndices);
    layout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout[3].InstanceDataStepRate = 0;

    layout[4].SemanticName = "BLENDWEIGHT";
    layout[4].SemanticIndex = 0;
    layout[4].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    layout[4].InputSlot = 0;
    layout[4].AlignedByteOffset = offsetof(Vertex, boneWeights);
    layout[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout[4].InstanceDataStepRate = 0;

    RETURN_IF_FAILED(device->CreateInputLayout(layout, 5, shaderBlob->GetBufferPointer(), 
        shaderBlob->GetBufferSize(), &inputLayout));


    return S_OK;
}

HRESULT Renderer::CreateBoneMatrixBuffer()
{
    D3D11_BUFFER_DESC bufferDesc = {};

    bufferDesc.ByteWidth = sizeof(BoneMatrixBuffer);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    RETURN_IF_FAILED(device->CreateBuffer(&bufferDesc, nullptr, &boneMatrixBuffer));

    return S_OK;
}

HRESULT Renderer::HandleShaderCompileResult(HRESULT hr, ID3DBlob*& errorBlob, ID3DBlob*& shaderBlob)
{
    if (FAILED(hr)) {
        if (errorBlob) {
            Logger::Error("Renderer", static_cast<const char*>(errorBlob->GetBufferPointer()));
            errorBlob->Release();
            errorBlob = nullptr;
        }
        if (shaderBlob) {
            shaderBlob->Release();
            shaderBlob = nullptr;
        }
        return hr;
    }
    else if (errorBlob) {
        Logger::Warning("Renderer", static_cast<const char*>(errorBlob->GetBufferPointer()));
        errorBlob->Release();
        errorBlob = nullptr;
    }

    return hr;
}

HRESULT Renderer::CreateVertexShader()
{
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(Paths::VertexShader, nullptr,
        nullptr, "main", "vs_5_0", 0, 0, &shaderBlob, &errorBlob);


    RETURN_IF_FAILED(HandleShaderCompileResult(hr, errorBlob, shaderBlob));

    RETURN_IF_FAILED_AND_RELEASE(device->CreateVertexShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &vertexShader), shaderBlob);

    RETURN_IF_FAILED_AND_RELEASE(CreateInputLayout(shaderBlob), shaderBlob);
    shaderBlob->Release();

    BindVertexStage();

    return S_OK;
}


HRESULT Renderer::CreatePixelShader()
{
    ID3DBlob* errorBlob = nullptr;
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(Paths::PixelShader, nullptr,
        nullptr, "main", "ps_5_0", 0, 0, &shaderBlob, &errorBlob);

    RETURN_IF_FAILED(HandleShaderCompileResult(hr, errorBlob, shaderBlob));

    RETURN_IF_FAILED_AND_RELEASE(device->CreatePixelShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &pixelShader), shaderBlob);
    shaderBlob->Release();

    BindPixelStage();

    return S_OK;
}

HRESULT Renderer::CreateDepthBuffer(HWND hwnd)
{
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

// TODO: MatrixBuffer를 PerFrame(View/Projection)과
//       PerObject(World) Constant Buffer로 분리
void Renderer::UpdateMatrixBuffer(const DirectX::XMMATRIX& world, const DirectX::XMMATRIX& meshNode, const Camera& camera)
{
    MatrixBuffer matrixData = {};

    matrixData.world = DirectX::XMMatrixTranspose(world);
    matrixData.view = DirectX::XMMatrixTranspose(camera.GetView());
    matrixData.projection = DirectX::XMMatrixTranspose(camera.GetProjection());
    matrixData.meshNode = DirectX::XMMatrixTranspose(meshNode);

    context->UpdateSubresource(matrixBuffer, 0, nullptr,
        &matrixData, 0, 0);
}

void Renderer::UpdateTransformBuffer(const Transform& transform, const DirectX::XMMATRIX& meshNode, const Camera& camera)
{
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

    UpdateMatrixBuffer(scale * rotation * translation, meshNode, camera);
}

void Renderer::UpdateMaterialBuffer(const Material& material)
{
    MaterialBuffer materialData = {};

    materialData.baseColor = material.GetBaseColor();
    if (material.GetTextureView())
        materialData.hasTexture = 1;

    context->UpdateSubresource(materialBuffer, 0, nullptr,
        &materialData, 0, 0);
}

void Renderer::UpdateLightBuffer()
{
    // TODO: Light 데이터를 Renderer 내부 하드코딩이 아닌
    //       Scene/Light 객체에서 전달하도록 변경
    LightBuffer lightData = {};

    lightData.direction = { 1.0f, -1.0f, 1.0f };
    lightData.intensity = 1.0f;
    lightData.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lightData.ambientIntensity = 0.2f;

    context->UpdateSubresource(lightBuffer, 0, nullptr,
        &lightData, 0, 0);
}

void Renderer::UpdateBoneMatrixBuffer(const std::vector<DirectX::XMFLOAT4X4>& matrices)
{
    BoneMatrixBuffer data = {};

    for (size_t i = 0; i < MAX_BONES; ++i)
        DirectX::XMStoreFloat4x4(&data.bones[i],DirectX::XMMatrixIdentity());

    for (size_t i = 0; i < matrices.size() && i < MAX_BONES; ++i)
    {
        DirectX::XMMATRIX matrix =
            DirectX::XMLoadFloat4x4(&matrices[i]);

        matrix = DirectX::XMMatrixTranspose(matrix);

        DirectX::XMStoreFloat4x4(&data.bones[i], matrix);
    }
    context->UpdateSubresource(boneMatrixBuffer, 0, nullptr,
        &data, 0, 0);
}

void Renderer::ClearFrame() {
    float rgbaColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    context->ClearRenderTargetView(renderTargetView, rgbaColor);

    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::BeginFrame()
{
    ClearFrame();
}

void Renderer::Draw(const Scene& scene, const Camera& camera)
{
    UpdateLightBuffer();

    for (std::size_t i = 0; i < scene.size(); ++i)
    {
        const GameObject* object = scene[i];

        UpdateBoneMatrixBuffer(object->animator.GetSkinningMatrices());

        const std::vector<Mesh>& meshes = object->model.GetMeshes();
        const std::vector<Material>& materials = object->model.GetMaterials();

        for (unsigned int meshIndex = 0; meshIndex < meshes.size(); ++meshIndex)
        {
            int nodeIndex = object->model.FindMeshNodeIndex(meshIndex);
            if (nodeIndex == -1)
            {
                Logger::Error("Renderer", "Mesh owner node not found");
                continue;
            }

            const DirectX::XMFLOAT4X4* nodeGlobalTransform = object->animator.GetNodeGlobalTransform(nodeIndex);
            if (nodeGlobalTransform == nullptr)
            {
                Logger::Error("Renderer", "Node global transform not found");
                continue;
            }

            DirectX::XMMATRIX meshNode = DirectX::XMLoadFloat4x4(nodeGlobalTransform);
            UpdateTransformBuffer(object->transform, meshNode, camera);

            const Mesh& mesh = meshes[meshIndex];
            const Material& material = materials[mesh.GetMaterialIndex()];
            ID3D11ShaderResourceView* textureView = material.GetTextureView();
            ID3D11SamplerState* samplerState = material.GetSamplerState();

            UpdateMaterialBuffer(material);

            context->PSSetShaderResources(0, 1, &textureView);
            context->PSSetSamplers(0, 1, &samplerState);

            UINT offset = 0;
            UINT stride = sizeof(Vertex);
            ID3D11Buffer* vertexBuffer = mesh.GetVertexBuffer();

            context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
            context->IASetIndexBuffer(mesh.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
            context->DrawIndexed(mesh.GetIndexCount(), 0, 0);
        }
    }
}

void Renderer::EndFrame()
{
    swapChain->Present(1, 0);
}
