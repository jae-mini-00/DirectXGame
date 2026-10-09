#include "Mesh.h"
#include "DxUtils.h"

Mesh::Mesh() : vertexBuffer(nullptr), indexBuffer(nullptr), indexCount(0) {}

Mesh::~Mesh()
{
    if (indexBuffer)
        indexBuffer->Release();

    if (vertexBuffer)
        vertexBuffer->Release();
}

ID3D11Buffer* Mesh::GetVertexBuffer() const { return vertexBuffer; }

ID3D11Buffer* Mesh::GetIndexBuffer() const { return indexBuffer; }

UINT Mesh::GetIndexCount() const { return indexCount; }

HRESULT Mesh::Initialize(ID3D11Device* device, const Vertex* vertices, UINT vertexCount,
    const unsigned int* indices, UINT indexCount) {
    RETURN_IF_FAILED(CreateVertexBuffer(device, vertices, vertexCount));
    RETURN_IF_FAILED_AND_RELEASE(CreateIndexBuffer(device, indices, indexCount), vertexBuffer);
    this->indexCount = indexCount;

    return S_OK;
}


HRESULT Mesh::CreateVertexBuffer(ID3D11Device* device, const Vertex* vertices, UINT count) {
    D3D11_BUFFER_DESC bufferDesc = {};
    D3D11_SUBRESOURCE_DATA initData = {};

    initData.pSysMem = vertices;
    bufferDesc.ByteWidth = sizeof(Vertex) * count;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    RETURN_IF_FAILED(device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer));

    return S_OK;
}

HRESULT Mesh::CreateIndexBuffer(ID3D11Device* device, const unsigned int* indices, UINT count) {
    D3D11_BUFFER_DESC bufferDesc = {};
    D3D11_SUBRESOURCE_DATA initData = {};

    initData.pSysMem = indices;
    bufferDesc.ByteWidth = sizeof(unsigned int) * count;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    RETURN_IF_FAILED(device->CreateBuffer(&bufferDesc, &initData, &indexBuffer));

    return S_OK;
}
