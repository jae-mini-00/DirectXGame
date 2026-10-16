#include "Graphics/Mesh.h"
#include "Core/DxUtils.h"

ID3D11Buffer* Mesh::GetVertexBuffer() const { return vertexBuffer; }
ID3D11Buffer* Mesh::GetIndexBuffer() const { return indexBuffer; }
UINT Mesh::GetIndexCount() const { return indexCount; }
unsigned int Mesh::GetMaterialIndex() const { return materialIndex; }

Mesh::Mesh() : vertexBuffer(nullptr), indexBuffer(nullptr), indexCount(0), materialIndex(0) {}

Mesh::~Mesh()
{
    if (indexBuffer)
        indexBuffer->Release();

    if (vertexBuffer)
        vertexBuffer->Release();
}

Mesh::Mesh(Mesh&& other) noexcept : vertexBuffer(other.vertexBuffer),
    indexBuffer(other.indexBuffer), indexCount(other.indexCount),
    materialIndex(other.materialIndex)
{
    other.vertexBuffer = nullptr;
    other.indexBuffer = nullptr;
    other.indexCount = 0;
    other.materialIndex = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        if (indexBuffer)
            indexBuffer->Release();
        if (vertexBuffer)
            vertexBuffer->Release();

        vertexBuffer = other.vertexBuffer;
        indexBuffer = other.indexBuffer;
        indexCount = other.indexCount;
        materialIndex = other.materialIndex;

        other.vertexBuffer = nullptr;
        other.indexBuffer = nullptr;
        other.indexCount = 0;
        other.materialIndex = 0;
    }

    return *this;
}

HRESULT Mesh::Initialize(ID3D11Device* device, const MeshData& data)
{
    RETURN_IF_FAILED(CreateVertexBuffer(device, data.vertices.data(), 
        static_cast<UINT>(data.vertices.size())));

    RETURN_IF_FAILED_AND_RELEASE(CreateIndexBuffer(device, data.indices.data(), 
        static_cast<UINT>(data.indices.size())), vertexBuffer);

    indexCount = static_cast<UINT>(data.indices.size());
    materialIndex = data.materialIndex;

    return S_OK;
}


HRESULT Mesh::CreateVertexBuffer(ID3D11Device* device, const Vertex* vertices, UINT count)
{
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

HRESULT Mesh::CreateIndexBuffer(ID3D11Device* device, const unsigned int* indices, UINT count)
{
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
