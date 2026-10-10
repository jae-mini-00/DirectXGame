#pragma once

#include <d3d11.h>

struct Vertex {
    float x, y, z;
    float r, g, b, a;
    float u, v;
};

class Mesh {
    public:
        Mesh();
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        HRESULT Initialize(ID3D11Device* device, const Vertex* vertices,
            UINT vertexCount, const unsigned int* indices, UINT indexCount);

        ID3D11Buffer* GetVertexBuffer() const;
        ID3D11Buffer* GetIndexBuffer() const;
        UINT GetIndexCount() const;

    private:
        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;
        UINT indexCount;

        HRESULT CreateVertexBuffer(ID3D11Device* device,
            const Vertex* vertices, UINT count);

        HRESULT CreateIndexBuffer(ID3D11Device* device,
            const unsigned int* indices, UINT count);
};