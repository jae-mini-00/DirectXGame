#include "Graphics/Model.h"
#include <utility>

const std::vector<Mesh>& Model::GetMeshes() const { return meshes; }
const std::vector<Material>& Model::GetMaterials() const { return materials; }

HRESULT Model::Initialize(ID3D11Device* device,
    ID3D11DeviceContext* context, const ModelData& data)
{
    meshes.clear();
    materials.clear();

    for (const MeshData& meshData : data.meshes)
    {
        Mesh mesh;

        HRESULT hr = mesh.Initialize(device, meshData);

        if (FAILED(hr))
            return hr;

        meshes.push_back(std::move(mesh));
    }

    for (const MaterialData& materialData : data.materials)
    {
        Material material;

        HRESULT hr = material.Initialize(device,
            context, materialData);

        if (FAILED(hr))
            return hr;

        materials.push_back(std::move(material));
    }

    return S_OK;
}