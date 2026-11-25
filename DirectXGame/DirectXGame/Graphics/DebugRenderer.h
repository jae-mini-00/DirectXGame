#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class Camera;
class Scene;

// Draws wireframe AABBs for scene GameObjects.
// Uses Collision::ToWorldAABB for position/scale; rotation is not yet supported.
class DebugRenderer final
{
public:
    HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void DrawColliders(const Scene& scene, const Camera& camera) const;

private:
    struct DebugVertex;

    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> matrixBuffer_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState_;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;
};
