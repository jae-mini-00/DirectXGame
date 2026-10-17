#pragma once

#include "Graphics/Mesh.h"
#include "Graphics/Material.h"

#include <assimp/scene.h>
#include <filesystem>
#include <vector>

struct ModelData
{
	std::vector<MeshData> meshes;
	std::vector<MaterialData> materials;
};

class ModelLoader 
{
	public:
		static bool Load(std::filesystem::path filePath, ModelData& outModel);

	private:
		static void LoadMaterials(const aiScene* scene, ModelData& outModel, 
			const std::filesystem::path& parentPath);
		static void LoadMeshes(const aiScene* scene, ModelData& outModel);
		static MeshData LoadMesh(const aiMesh* mesh);
};