#include "Graphics/DebugRenderer.h"

#include "Core/Camera.h"
#include "Scene/GameObject.h"
#include "Scene/Scene.h"
#include "Physics/Collision.h"

#include <DirectXMath.h>
#include <d3dcompiler.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

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

    // Renderer::Draw() relies on its D3D11 state remaining bound between frames.
    // Preserve every pipeline slot changed by this debug pass.
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
        ID3D11DeviceContext* context_ = nullptr; // Borrowed, not owned.
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

    bool IsValidWorldAABB(const WorldAABB& box)
    {
        const auto& center = box.center;
        const auto& extents = box.halfExtents;
        return std::isfinite(center.x) && std::isfinite(center.y) &&
            std::isfinite(center.z) &&
            std::isfinite(extents.x) && std::isfinite(extents.y) &&
            std::isfinite(extents.z) &&
            extents.x >= 0.0f && extents.y >= 0.0f && extents.z >= 0.0f;
    }
    using Point = DirectX::XMFLOAT3;
    using Lines = std::vector<Point>;
    constexpr int kSegments = 32;
    constexpr float kPi = 3.14159265358979323846f;
    // A capsule has two rings, four semicircles and four straight edges.
    constexpr size_t kMaxWireVertices = 8 * kSegments + 8;

    void AddArc(Lines& lines, Point center, Point u, Point v, float radius,
        int segments, float angle)
    {
        Point previous{};
        for (int i = 0; i <= segments; ++i)
        {
            const float t = angle * static_cast<float>(i) / segments;
            const float x = radius * std::cos(t);
            const float y = radius * std::sin(t);
            const Point current{center.x + x*u.x + y*v.x,
                center.y + x*u.y + y*v.y, center.z + x*u.z + y*v.z};
            if (i > 0)
            {
                lines.push_back(previous);
                lines.push_back(current);
            }
            previous = current;
        }
    }

    void AddSphere(Lines& lines, Point center, float radius)
    {
        const Point x{1,0,0}, y{0,1,0}, z{0,0,1};
        AddArc(lines, center, x, y, radius, kSegments, 2*kPi);
        AddArc(lines, center, y, z, radius, kSegments, 2*kPi);
        AddArc(lines, center, z, x, radius, kSegments, 2*kPi);
    }

    void AddCapsule(Lines& lines, const WorldCapsule& cap)
    {
        const Point a = cap.pointA, b = cap.pointB;
        const Point d{b.x-a.x, b.y-a.y, b.z-a.z};
        const float length = std::hypot(d.x, d.y, d.z);
        if (length == 0)
        {
            AddSphere(lines, a, cap.radius);
            return;
        }

        const Point axis{d.x/length, d.y/length, d.z/length};
        Point u = std::abs(axis.x) < 0.8f
            ? Point{0, axis.z, -axis.y} : Point{-axis.z, 0, axis.x};
        const float norm = std::hypot(u.x, u.y, u.z);
        u = {u.x/norm, u.y/norm, u.z/norm};
        const Point v{axis.y*u.z-axis.z*u.y, axis.z*u.x-axis.x*u.z,
            axis.x*u.y-axis.y*u.x};
        AddArc(lines, a, u, v, cap.radius, kSegments, 2*kPi);
        AddArc(lines, b, u, v, cap.radius, kSegments, 2*kPi);
        const Point backwards{-axis.x, -axis.y, -axis.z};
        for (Point side : {u, v})
        {
            AddArc(lines, a, side, backwards, cap.radius, kSegments/2, kPi);
            AddArc(lines, b, side, axis, cap.radius, kSegments/2, kPi);
            for (float offset : {cap.radius, -cap.radius})
            {
                lines.push_back({a.x+side.x*offset, a.y+side.y*offset, a.z+side.z*offset});
                lines.push_back({b.x+side.x*offset, b.y+side.y*offset, b.z+side.z*offset});
            }
        }
    }

    void AddBox(Lines& lines, const WorldAABB& box)
    {
        if (!IsValidWorldAABB(box))
            return;
        const auto& c = box.center;
        const auto& h = box.halfExtents;
        std::array<Point, 8> corners{};
        for (unsigned i = 0; i < corners.size(); ++i)
            corners[i] = {c.x + ((i & 1) ? h.x : -h.x),
                c.y + ((i & 2) ? h.y : -h.y), c.z + ((i & 4) ? h.z : -h.z)};
        for (auto edge : kBoxEdges)
        {
            lines.push_back(corners[edge[0]]);
            lines.push_back(corners[edge[1]]);
        }
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
    vertexDesc.ByteWidth = static_cast<UINT>(sizeof(DebugVertex) * kMaxWireVertices);
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

    // Make the debug box visible even when its lines are inside a model.
    // Disable depth writes too, so this overlay cannot affect subsequent rendering.
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

    struct Entry
    {
        WorldCollider shape;
        bool colliding = false;
    };
    std::vector<Entry> entries;
    entries.reserve(scene.size());
    for (std::size_t i = 0; i < scene.size(); ++i)
    {
        const auto* object = scene[i];
        if (object && !std::holds_alternative<std::monostate>(object->collider))
            entries.push_back({Collision::ToWorldCollider(object->collider,
                object->transform.position, object->transform.scale)});
    }

    for (std::size_t i = 0; i < entries.size(); ++i)
        for (std::size_t j = i + 1; j < entries.size(); ++j)
            if (Collision::Intersects(entries[i].shape, entries[j].shape))
                entries[i].colliding = entries[j].colliding = true;

    Lines lines;
    lines.reserve(kMaxWireVertices);
    for (const Entry& entry : entries)
    {
        lines.clear();
        if (const auto* box = std::get_if<WorldAABB>(&entry.shape))
            AddBox(lines, *box);
        else if (const auto* sphere = std::get_if<WorldSphere>(&entry.shape))
            AddSphere(lines, sphere->center, sphere->radius);
        else if (const auto* capsule = std::get_if<WorldCapsule>(&entry.shape))
            AddCapsule(lines, *capsule);
        if (lines.empty())
            continue;

        const DirectX::XMFLOAT4 color = entry.colliding
            ? DirectX::XMFLOAT4{1.0f, 0.1f, 0.1f, 1.0f}
            : DirectX::XMFLOAT4{0.05f, 0.75f, 0.10f, 1.0f};

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        HRESULT hr = context->Map(vertexBuffer_.Get(), 0,
            D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (FAILED(hr))
            break;

        auto* output = static_cast<DebugVertex*>(mapped.pData);
        for (const Point& point : lines)
            *output++ = {point, color};
        context->Unmap(vertexBuffer_.Get(), 0);
        context->Draw(static_cast<UINT>(lines.size()), 0);
    }
}
