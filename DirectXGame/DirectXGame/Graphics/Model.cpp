#include "Graphics/Model.h"
#include "Core/Logger/Logger.h"
#include <utility>

const std::vector<Mesh>& Model::GetMeshes() const { return meshes; }
const std::vector<Material>& Model::GetMaterials() const { return materials; }
const SkeletonData& Model::GetSkeleton() const { return skeleton; }
const NodeHierarchyData& Model::GetNodeHierarchy() const { return nodeHierarchy; }
const std::vector<AnimationClip>& Model::GetAnimationClip() const { return animationClip; }


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

    meshIndexs = data.meshIndexs;
    skeleton = data.skeleton;
    nodeHierarchy = data.nodeHierarchy;
    animationClip = data.animationClips;
    ResolveMeshIndices();

    return S_OK;
}

void Model::ResolveMeshIndices()
{
    for (unsigned int i = 0; i < nodeHierarchy.nodes.size(); ++i)
    {
        for (unsigned int j = 0; j < nodeHierarchy.nodes[i].meshIndexs.size();)
        {
            int meshindex = FindMeshIndex(nodeHierarchy.nodes[i].meshIndexs[j]);

            if (meshindex == -1)
                nodeHierarchy.nodes[i].meshIndexs.erase(nodeHierarchy.nodes[i].meshIndexs.begin() + j);
            else
            {
                nodeHierarchy.nodes[i].meshIndexs[j] = meshindex;
                ++j;
            }
        }
    }
}

int Model::FindMeshIndex(unsigned int originalMeshIndex) const
{
    for (unsigned int i = 0; i < meshIndexs.size(); ++i)
    {
        if (meshIndexs[i] == originalMeshIndex)
            return i;
    }

    return -1;
}

int Model::FindMeshNodeIndex(unsigned int meshIndex) const
{
    for (unsigned int i = 0; i < nodeHierarchy.nodes.size(); ++i)
    {
        for (unsigned int j = 0; j < nodeHierarchy.nodes[i].meshIndexs.size(); ++j)
        {
            if (nodeHierarchy.nodes[i].meshIndexs[j] == meshIndex)
                return i;
        }
    }

    return -1;
}
