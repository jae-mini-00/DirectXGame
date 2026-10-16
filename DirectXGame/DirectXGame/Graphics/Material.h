#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <string>

struct MaterialData
{
    DirectX::XMFLOAT4 baseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    std::string texturePath;
};

class Material
{
    public:
        Material();
        ~Material();
        Material(Material&& other) noexcept;
        Material& operator=(Material&& other) noexcept;

        HRESULT Initialize(ID3D11Device* device,
            ID3D11DeviceContext* context, const MaterialData& data);
        ID3D11ShaderResourceView* GetTextureView() const;
        ID3D11SamplerState* GetSamplerState() const;
        const DirectX::XMFLOAT4& GetBaseColor() const;

    private:
        DirectX::XMFLOAT4 baseColor;
        ID3D11ShaderResourceView* textureView;
        ID3D11SamplerState* samplerState;
};