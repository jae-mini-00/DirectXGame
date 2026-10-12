#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Material.h"

struct ModelData
{
	std::vector<MeshData> meshes;
	std::vector<MaterialData> materials;
};

class ModelLoader 
{
	public:
		static bool Load(const char* filePath, ModelData& outModel);

	private:
		static void LoadMaterials(const aiScene* scene, ModelData& outModel);
		static void LoadMeshes(const aiScene* scene, ModelData& outModel);
		static MeshData LoadMesh(const aiMesh* mesh);
};