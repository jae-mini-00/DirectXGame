#pragma once

#include <vector>
#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/ModelLoader.h"

class Model
{
    public:
        const std::vector<Mesh>& GetMeshes() const;
        const std::vector<Material>& GetMaterials() const;
        const SkeletonData& GetSkeleton() const;
        const NodeHierarchyData& GetNodeHierarchy() const;
        const std::vector<AnimationClip>& GetAnimationClip() const;

        Model() = default;
        ~Model() = default;
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        int FindMeshNodeIndex(unsigned int meshIndex) const;

        HRESULT Initialize(ID3D11Device* device,
            ID3D11DeviceContext* context, const ModelData& data);

    private:
        std::vector<Mesh> meshes;
        std::vector<unsigned int> meshIndexs;
        std::vector<Material> materials;
        std::vector<AnimationClip> animationClip;
        SkeletonData skeleton;
        NodeHierarchyData nodeHierarchy;

        void ResolveMeshIndices();
        int FindMeshIndex(unsigned int originalMeshIndex) const;

};