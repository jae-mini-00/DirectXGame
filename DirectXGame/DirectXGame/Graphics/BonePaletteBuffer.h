#pragma once

#include <d3d11.h> 
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>

class BonePaletteBuffer
{
    public:
        HRESULT Update(ID3D11Device* device, ID3D11DeviceContext* context, const std::vector<DirectX::XMFLOAT4X4>& matrices);
        void Bind(ID3D11DeviceContext* context) const;

    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        size_t capacity = 0;
        size_t activeCount = 0;
        
        HRESULT EnsureCapacity(ID3D11Device* device, size_t requiredCount);
};