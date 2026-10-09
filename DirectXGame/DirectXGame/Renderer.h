#pragma once

#include "GameObject.h"
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")

struct FeatureLevelInfo
{
    D3D_FEATURE_LEVEL levels[1];
    UINT count;
    D3D_FEATURE_LEVEL selectedLevel;
};

struct MatrixBuffer
{
    DirectX::XMMATRIX world;
    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    HRESULT Initialize(HWND hwnd);
    void BeginFrame();
    void Draw(const GameObject& object);
    void EndFrame();
    ID3D11Device* GetDevice() const;
private:
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11InputLayout* inputLayout;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11Buffer* constantBuffer;
    ID3D11Texture2D* depthBuffer;
    ID3D11DepthStencilView* depthStencilView;

    DirectX::XMMATRIX view;
    DirectX::XMMATRIX projection;

    void BindVertexStage();
    DXGI_SWAP_CHAIN_DESC CreateswapChainDescInfo(HWND hwnd);
    FeatureLevelInfo CreateFeatureLevelInfo();
    D3D11_VIEWPORT CreateViewport(HWND hwnd);
    HRESULT CreateRenderTarget();
    HRESULT CreateInputLayout(ID3DBlob* shaderBlob);
    HRESULT CreateVertexShader();
    HRESULT CreatePixelShader();
    HRESULT CreateConstantBuffer();
    HRESULT CreateDepthBuffer(HWND hwnd);
    DirectX::XMMATRIX CreateTransform(const Transform& transform);

    void CreateCameraMatrices(HWND hwnd);
    void UpdateMatrixBuffer(const DirectX::XMMATRIX& world);
    void UpdateTransformBuffer(const Transform& transform);
    void Clear();
};