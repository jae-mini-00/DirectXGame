#pragma once

#include <d3d11.h>
#include <vector>

struct Vertex
{
    float x, y, z;
    float u, v;
    float nx, ny, nz;
};

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int materialIndex = 0;
};

class Mesh
{
    public:
        Mesh();
        ~Mesh();
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        HRESULT Initialize(ID3D11Device* device, const MeshData& data);

        ID3D11Buffer* GetVertexBuffer() const;
        ID3D11Buffer* GetIndexBuffer() const;
        UINT GetIndexCount() const;
        unsigned int GetMaterialIndex() const;

    private:
        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;
        UINT indexCount;
        unsigned int materialIndex;

        HRESULT CreateVertexBuffer(ID3D11Device* device,
            const Vertex* vertices, UINT count);

        HRESULT CreateIndexBuffer(ID3D11Device* device,
            const unsigned int* indices, UINT count);
};