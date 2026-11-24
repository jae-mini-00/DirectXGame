#include "Graphics/DebugRenderer.h"

#include "Core/Camera.h"
#include "Scene/GameObject.h"
#include "Scene/Scene.h"

#include <DirectXMath.h>
#include <d3dcompiler.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>

#pragma comment(lib, "d3dcompiler.lib")

namespace
{
    constexpr char kDebugShader[] = R"(
        cbuffer DebugMatrices : register(b0)
        {
            float4x4 viewProjection;
        };

        struct VSInput
        {
            float3 position : POSITION;
            float4 color : COLOR;
        };

        struct VSOutput
        {
            float4 position : SV_POSITION;
            float4 color : COLOR;
        };

        VSOutput VSMain(VSInput input)
        {
            VSOutput output;
            output.position = mul(float4(input.position, 1.0f), viewProjection);
            output.color = input.color;
            return output;
        }

        float4 PSMain(VSOutput input) : SV_TARGET
        {
            return input.color;
        }
    )";

    struct MatrixConstants
    {
        DirectX::XMFLOAT4X4 viewProjection;
    };

    static_assert(sizeof(MatrixConstants) % 16 == 0,
        "D3D11 constant buffers require 16-byte alignment in size.");

    class ScopedDebugState
    {
    public:
        explicit ScopedDebugState(ID3D11DeviceContext* context) : context_(context)
        {
            context_->IAGetPrimitiveTopology(&topology_);
            context_->IAGetInputLayout(inputLayout_.GetAddressOf());
            context_->IAGetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(),
                &vertexStride_, &vertexOffset_);
            context_->VSGetShader(vertexShader_.GetAddressOf(), nullptr, nullptr);
            context_->PSGetShader(pixelShader_.GetAddressOf(), nullptr, nullptr);
            context_->VSGetConstantBuffers(0, 1, vsBuffer_.GetAddressOf());
            context_->OMGetDepthStencilState(depthState_.GetAddressOf(), &stencilRef_);
            context_->RSGetState(rasterizerState_.GetAddressOf());
        }

        ~ScopedDebugState()
        {
            ID3D11Buffer* previousVertexBuffer = vertexBuffer_.Get();
            ID3D11Buffer* previousVSBuffer = vsBuffer_.Get();

            context_->IASetPrimitiveTopology(topology_);
            context_->IASetInputLayout(inputLayout_.Get());
            context_->IASetVertexBuffers(0, 1, &previousVertexBuffer,
                &vertexStride_, &vertexOffset_);
            context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
            context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
            context_->VSSetConstantBuffers(0, 1, &previousVSBuffer);
            context_->OMSetDepthStencilState(depthState_.Get(), stencilRef_);
            context_->RSSetState(rasterizerState_.Get());
        }

        ScopedDebugState(const ScopedDebugState&) = delete;
        ScopedDebugState& operator=(const ScopedDebugState&) = delete;

    private:
        ID3D11DeviceContext* context_ = nullptr;
        D3D11_PRIMITIVE_TOPOLOGY topology_ = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        UINT vertexStride_ = 0;
        UINT vertexOffset_ = 0;
        UINT stencilRef_ = 0;

        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> vsBuffer_;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState_;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;
    };

    constexpr std::array<std::array<unsigned int, 2>, 12> kBoxEdges = {{
        {{0, 1}}, {{1, 3}}, {{3, 2}}, {{2, 0}},
        {{4, 5}}, {{5, 7}}, {{7, 6}}, {{6, 4}},
        {{0, 4}}, {{1, 5}}, {{2, 6}}, {{3, 7}}
    }};

    bool IsValidHalfExtents(const DirectX::XMFLOAT3& extents)
    {
        return std::isfinite(extents.x) && std::isfinite(extents.y) &&
            std::isfinite(extents.z) &&
            extents.x >= 0.0f && extents.y >= 0.0f && extents.z >= 0.0f;
    }
}

struct DebugRenderer::DebugVertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

HRESULT DebugRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    if (!device || !context)
        return E_INVALIDARG;

    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;

    HRESULT hr = D3DCompile(kDebugShader, sizeof(kDebugShader) - 1,
        "DebugRenderer", nullptr, nullptr, "VSMain", "vs_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS, 0, vsBlob.GetAddressOf(), errors.GetAddressOf());
    if (FAILED(hr))
    {
        if (errors)
            OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));
        return hr;
    }

    errors.Reset();
    hr = D3DCompile(kDebugShader, sizeof(kDebugShader) - 1,
        "DebugRenderer", nullptr, nullptr, "PSMain", "ps_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS, 0, psBlob.GetAddressOf(), errors.GetAddressOf());
    if (FAILED(hr))
    {
        if (errors)
            OutputDebugStringA(static_cast<const char*>(errors->GetBufferPointer()));
        return hr;
    }

    hr = device->CreateVertexShader(vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(), nullptr, vertexShader_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    hr = device->CreatePixelShader(psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(), nullptr, pixelShader_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    const D3D11_INPUT_ELEMENT_DESC inputElements[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
            static_cast<UINT>(offsetof(DebugVertex, position)), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
            static_cast<UINT>(offsetof(DebugVertex, color)), D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    hr = device->CreateInputLayout(inputElements, ARRAYSIZE(inputElements),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        inputLayout_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    D3D11_BUFFER_DESC vertexDesc = {};
    vertexDesc.ByteWidth = static_cast<UINT>(sizeof(DebugVertex) * 24);
    vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = device->CreateBuffer(&vertexDesc, nullptr, vertexBuffer_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    D3D11_BUFFER_DESC matrixDesc = {};
    matrixDesc.ByteWidth = sizeof(MatrixConstants);
    matrixDesc.Usage = D3D11_USAGE_DEFAULT;
    matrixDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&matrixDesc, nullptr, matrixBuffer_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = FALSE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    hr = device->CreateDepthStencilState(&depthDesc, depthState_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.DepthClipEnable = TRUE;
    hr = device->CreateRasterizerState(&rasterDesc, rasterizerState_.ReleaseAndGetAddressOf());
    if (FAILED(hr)) return hr;

    context_ = context;
    return S_OK;
}

void DebugRenderer::DrawColliders(const Scene& scene, const Camera& camera) const
{
    if (!context_ || !vertexBuffer_ || !matrixBuffer_)
        return;

    ID3D11DeviceContext* context = context_.Get();
    ScopedDebugState previousState(context);

    MatrixConstants matrices = {};
    DirectX::XMStoreFloat4x4(&matrices.viewProjection,
        DirectX::XMMatrixTranspose(camera.GetView() * camera.GetProjection()));
    context->UpdateSubresource(matrixBuffer_.Get(), 0, nullptr, &matrices, 0, 0);

    ID3D11Buffer* vertexBuffer = vertexBuffer_.Get();
    ID3D11Buffer* matrixBuffer = matrixBuffer_.Get();
    const UINT stride = sizeof(DebugVertex);
    const UINT offset = 0;

    context->IASetInputLayout(inputLayout_.Get());
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->VSSetShader(vertexShader_.Get(), nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &matrixBuffer);
    context->PSSetShader(pixelShader_.Get(), nullptr, 0);
    context->OMSetDepthStencilState(depthState_.Get(), 0);
    context->RSSetState(rasterizerState_.Get());

    const DirectX::XMFLOAT4 color = {0.05f, 0.75f, 0.10f, 1.0f};

    for (std::size_t i = 0; i < scene.size(); ++i)
    {
        const GameObject* object = scene[i];
        if (!object || !IsValidHalfExtents(object->collider.halfExtents))
            continue;

        const auto& position = object->transform.position;
        const auto& collider = object->collider;

        const float cx = position.x + collider.center.x;
        const float cy = position.y + collider.center.y;
        const float cz = position.z + collider.center.z;
        const float hx = collider.halfExtents.x;
        const float hy = collider.halfExtents.y;
        const float hz = collider.halfExtents.z;

        const std::array<DirectX::XMFLOAT3, 8> corners = {{
            {cx - hx, cy - hy, cz - hz},
            {cx + hx, cy - hy, cz - hz},
            {cx - hx, cy + hy, cz - hz},
            {cx + hx, cy + hy, cz - hz},
            {cx - hx, cy - hy, cz + hz},
            {cx + hx, cy - hy, cz + hz},
            {cx - hx, cy + hy, cz + hz},
            {cx + hx, cy + hy, cz + hz}
        }};

        std::array<DebugVertex, 24> vertices = {};
        for (std::size_t edge = 0; edge < kBoxEdges.size(); ++edge)
        {
            vertices[edge * 2] = {corners[kBoxEdges[edge][0]], color};
            vertices[edge * 2 + 1] = {corners[kBoxEdges[edge][1]], color};
        }

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        HRESULT hr = context->Map(vertexBuffer_.Get(), 0,
            D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr))
            break;

        std::memcpy(mapped.pData, vertices.data(), sizeof(vertices));
        context->Unmap(vertexBuffer_.Get(), 0);
        context->Draw(static_cast<UINT>(vertices.size()), 0);
    }
}
