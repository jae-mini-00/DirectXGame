#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

struct ModelData
{
	std::vector<MeshData> meshes;
};

class ModelLoader 
{
	public:
		static bool Load(const char* filePath, ModelData& outModel);
};