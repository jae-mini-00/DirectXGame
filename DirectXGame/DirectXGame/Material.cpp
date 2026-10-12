#include "Material.h"
#include "DxUtils.h"

#include <directxtk/WICTextureLoader.h>
#include <string>
#include <Windows.h>

ID3D11ShaderResourceView* Material::GetTextureView() const { return textureView; }
ID3D11SamplerState* Material::GetSamplerState() const { return samplerState; }
const DirectX::XMFLOAT4& Material::GetBaseColor() const { return baseColor; }

Material::Material() : baseColor(1.0f, 1.0f, 1.0f, 1.0f),
textureView(nullptr), samplerState(nullptr) {}

Material::~Material()
{
    if (samplerState)
        samplerState->Release();

    if (textureView)
        textureView->Release();
}

Material::Material(Material&& other) noexcept : baseColor(other.baseColor),
    textureView(other.textureView), samplerState(other.samplerState)
{
    other.textureView = nullptr;
    other.samplerState = nullptr;
}

Material& Material::operator=(Material&& other) noexcept
{
    if (this != &other)
    {
        if (samplerState)
            samplerState->Release();
        if (textureView)
            textureView->Release();

        baseColor = other.baseColor;
        textureView = other.textureView;
        samplerState = other.samplerState;

        other.textureView = nullptr;
        other.samplerState = nullptr;
    }
    return *this;
}

static std::wstring ToWideString(const std::string& str)
{
    if (str.empty())
        return L"";

    int sizeNeeded = MultiByteToWideChar(
        CP_UTF8,
        0,
        str.c_str(),
        -1,
        nullptr,
        0
    );

    if (sizeNeeded == 0)
        return L"";

    std::wstring result(sizeNeeded, L'\0');

    MultiByteToWideChar(
        CP_UTF8,
        0,
        str.c_str(),
        -1,
        result.data(),
        sizeNeeded
    );

    result.pop_back();

    return result;
}

HRESULT Material::Initialize(ID3D11Device* device,
    ID3D11DeviceContext* context, const MaterialData& data)
{
    baseColor = data.baseColor;

    if (!data.texturePath.empty())
    {
        std::wstring texturePath =
            ToWideString(data.texturePath);

        RETURN_IF_FAILED(DirectX::CreateWICTextureFromFile(
                device, context, texturePath.c_str(),
                nullptr, &textureView));
    }

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    RETURN_IF_FAILED(device->CreateSamplerState(
        &samplerDesc, &samplerState));

    return S_OK;
}
