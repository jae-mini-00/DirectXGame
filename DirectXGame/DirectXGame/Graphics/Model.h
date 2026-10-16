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

        Model() = default;
        ~Model() = default;
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;
        HRESULT Initialize(ID3D11Device* device,
            ID3D11DeviceContext* context, const ModelData& data);

    private:
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
};