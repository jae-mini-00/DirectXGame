#pragma once

#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/ModelLoadResult.h"
#include "Graphics/AnimationClip.h"
#include "Graphics/NodeData.h"
#include "Graphics/SkeletonData.h"

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
	std::vector<unsigned int> meshIndexs;
	std::vector<AnimationClip> animationClips;

	SkeletonData skeleton;
	NodeHierarchyData nodeHierarchy;
};

class ModelLoader 
{
	public:
		static LoadResult Load(std::filesystem::path filePath, ModelData& outModel);

	private:
		
		static bool AddBoneInfluence(Vertex& vertex, int boneIndex, float weight);
		static bool AreMatricesEqual(const DirectX::XMFLOAT4X4& a, const DirectX::XMFLOAT4X4& b);
		static bool FillMissingPRS(AnimationChannel& channel, const NodeHierarchyData& hierarchy);
		static bool BuildDefaultNodeGlobals(const NodeHierarchyData& hierarchy, std::vector<DirectX::XMFLOAT4X4>& globals);
		static void CollectBoneBindGlobals(const aiScene* scene, const SkeletonData& skeleton, std::vector<DirectX::XMFLOAT4X4>& globals);
		static bool ApplyBindLocalTransforms(NodeHierarchyData& hierarchy, const std::vector<DirectX::XMFLOAT4X4>& globals);
		static aiMatrix4x4 CalculateNodeGlobalTransform(const aiNode* startNode);
		static aiNode* FindMeshOwnerNode(aiNode* node, unsigned int meshIndex);
		static DirectX::XMFLOAT4X4 ConvertMatrix(const aiMatrix4x4& matrix);
		static bool ResolveBoneNodes(SkeletonData& skeleton, const NodeHierarchyData& hierarchy);
		static bool ResolveBindPose(const aiScene* scene, const SkeletonData& skeleton, NodeHierarchyData& hierarchy);

		static bool LoadMesh(const aiMesh* mesh, unsigned int materialIndex, SkeletonData& skeleton, MeshData& outMesh);
		static bool LoadMaterials(const aiScene* scene, ModelData& outModel, const std::filesystem::path& parentPath);
		static int LoadMeshes(const aiScene* scene, ModelData& outModel);
		static void LoadVertices(const aiMesh* mesh, MeshData& meshData);
		static bool LoadBones(const aiMesh* mesh, MeshData& meshData, SkeletonData& skeleton);
		static bool LoadIndices(const aiMesh* mesh, MeshData& meshData);
		static bool LoadAnimations(const aiScene* scene, ModelData& outModel);
		static bool LoadNodes(const aiNode* node, NodeHierarchyData& hierarchy, int parentIndex);
};