#pragma once

#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/ModelLoadResult.h"
#include "Graphics/SkeletonData.h"
#include "Graphics/Animator.h"

#include <filesystem>
#include <vector>
#include <assimp/matrix4x4.h>

struct aiScene;
struct aiMesh;
struct aiBone;
struct aiNode;

struct ModelData
{
	std::vector<MeshData> meshes;
	std::vector<MaterialData> materials;
	std::vector<AnimationClip> animationClips;
	SkeletonData skeleton;
};

class ModelLoader 
{
	public:
		static LoadResult Load(std::filesystem::path filePath, ModelData& outModel);

	private:
		static void LoadMaterials(const aiScene* scene, ModelData& outModel, 
			const std::filesystem::path& parentPath);
		static int LoadMeshes(const aiScene* scene, ModelData& outModel);
		static MeshData LoadMesh(const aiMesh* mesh, unsigned int materialIndex, SkeletonData& skeleton);
		static bool AddBoneInfluence(Vertex& vertex, int boneIndex, float weight);
		static DirectX::XMFLOAT4X4 ConvertMatrix(const aiMatrix4x4& matrix);
		static void ResolveBoneParents(const aiNode* node, SkeletonData& skeleton, int parentBoneIndex);
		static void LoadVertices(const aiMesh* mesh, MeshData& meshData);
		static void LoadBones(const aiMesh* mesh, MeshData& meshData, SkeletonData& skeleton);
		static void LoadIndices(const aiMesh* mesh,	MeshData& meshData);
		static void LoadAnimations(const aiScene* scene, ModelData& outModel);
};