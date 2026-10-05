#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")

#define RETURN_IF_FAILED(expr) \
    do { \
        HRESULT hr = (expr); \
        if (FAILED(hr)) \
            return hr; \
    } while (0)

#define RETURN_IF_FAILED_AND_RELEASE(expr, resource) \
    do { \
        HRESULT hr = (expr); \
        if (FAILED(hr)) {\
            if (resource)\
                resource->Release();\
            return hr; \
            }\
    } while (0)

struct FeatureLevelInfo
{
    D3D_FEATURE_LEVEL levels[1];
    UINT count;
    D3D_FEATURE_LEVEL selectedLevel;
};

struct Vertex
{
    float x, y, z;
    float r, g, b, a;
};

class Renderer
{
public:
    Renderer();
    ~Renderer();

    HRESULT Initialize(HWND hwnd);
    void Render();
private:
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11InputLayout* inputLayout;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;

    void BindVertexStage();
    DXGI_SWAP_CHAIN_DESC CreateswapChainDescInfo(HWND hwnd);
    FeatureLevelInfo CreateFeatureLevelInfo();
    D3D11_VIEWPORT CreateViewport(HWND hwnd);
    HRESULT CreateRenderTarget();
    HRESULT CreateVertexBuffer(const Vertex* vertices, UINT count);
    HRESULT CreateIndexBuffer(const unsigned int* indices, UINT count);
    HRESULT CreateInputLayout(ID3DBlob* shaderBlob);
    HRESULT CreateVertexShader();
    HRESULT CreatePixelShader();
};