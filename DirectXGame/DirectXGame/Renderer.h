#include <windows.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

struct FeatureLevelInfo
{
    D3D_FEATURE_LEVEL levels[1];
    UINT count;
    D3D_FEATURE_LEVEL selectedLevel;
};

class Renderer
{
public:
    Renderer();
    ~Renderer();

    HRESULT Initialize(HWND hwnd);

private:
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;

    DXGI_SWAP_CHAIN_DESC CreateswapChainDescInfo(HWND hwnd);
    FeatureLevelInfo CreateFeatureLevelInfo();
};