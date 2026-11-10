#include "Graphics/BonePaletteBuffer.h"
#include "Core/DxUtils.h"

#include <utility>

HRESULT BonePaletteBuffer::EnsureCapacity(ID3D11Device* device, size_t requiredCount)
{
    constexpr size_t maxCount = UINT_MAX / sizeof(DirectX::XMFLOAT4X4);
    if (requiredCount <= capacity)
        return S_OK;
    if (requiredCount > maxCount)
        return E_INVALIDARG;

    size_t newCapacity = capacity == 0 ? 64 : capacity;

    while (newCapacity < requiredCount)
    {
        if (newCapacity > maxCount / 2)
        {
            newCapacity = requiredCount;
            break;
        }
        newCapacity *= 2;
    }

    Microsoft::WRL::ComPtr<ID3D11Buffer> newBuffer;
    D3D11_BUFFER_DESC bufferDesc = {};

    bufferDesc.ByteWidth = static_cast<UINT>(newCapacity * sizeof(DirectX::XMFLOAT4X4));
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(DirectX::XMFLOAT4X4);

    RETURN_IF_FAILED(device->CreateBuffer(&bufferDesc, nullptr, newBuffer.GetAddressOf()));

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> newSrv;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = static_cast<UINT>(newCapacity);

    RETURN_IF_FAILED(device->CreateShaderResourceView(newBuffer.Get(), &srvDesc, newSrv.GetAddressOf()));

    buffer = std::move(newBuffer);
    srv = std::move(newSrv);
    capacity = newCapacity;

    return S_OK;
}

HRESULT BonePaletteBuffer::Update(ID3D11Device* device, ID3D11DeviceContext* context, const std::vector<DirectX::XMFLOAT4X4>& matrices)
{
    activeCount = 0;
    if (matrices.empty())
        return S_OK;

    RETURN_IF_FAILED(EnsureCapacity(device, matrices.size()));

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    
    RETURN_IF_FAILED(context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));

    DirectX::XMFLOAT4X4* destination = static_cast<DirectX::XMFLOAT4X4*>(mapped.pData);

    for (size_t i = 0; i < matrices.size(); ++i)
        DirectX::XMStoreFloat4x4(&destination[i], DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&matrices[i])));

    context->Unmap(buffer.Get(), 0);
    activeCount = matrices.size();

    return S_OK;
}

void BonePaletteBuffer::Bind(ID3D11DeviceContext* context) const
{
    ID3D11ShaderResourceView* view = activeCount == 0 ? nullptr : srv.Get();

    context->VSSetShaderResources(0, 1, &view);
}