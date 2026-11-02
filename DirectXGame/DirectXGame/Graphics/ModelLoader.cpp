#include "Graphics/ModelLoader.h"
#include "Core/Logger/Logger.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

LoadResult ModelLoader::Load(std::filesystem::path filePath, ModelData& outModel)
{
    LoadResult lr = {true, LoadError::None, ""};
    Assimp::Importer importer;
    std::filesystem::path parent_path = filePath.parent_path();

    const aiScene* scene = importer.ReadFile(filePath.string(), 
        aiProcess_Triangulate | aiProcess_GenSmoothNormals);

    if (scene == nullptr)
    {
        lr.success = false;
        lr.error = LoadError::FileReadFailed;
        lr.message = importer.GetErrorString();

        return lr;
    }

    outModel.meshes.clear();
    outModel.materials.clear();
    outModel.skeleton.bones.clear();
    outModel.skeleton.nameToIndex.clear();

    LoadMaterials(scene, outModel, parent_path);
    if (LoadMeshes(scene, outModel) <= 0)
    {
        lr.success = false;
        lr.error = LoadError::NoValidMesh;
        lr.message = "Model contains no valid meshes.";
    }
    else if (scene->mRootNode != nullptr)
        ResolveBoneParents(scene->mRootNode,
            outModel.skeleton, -1);

    return lr;
}

int ModelLoader::LoadMeshes(const aiScene* scene, ModelData& outModel)
{
    int success = 0;
    unsigned int DefaultMateriaIndex = static_cast<unsigned int>(outModel.materials.size()) - 1;

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        unsigned int resolvedMaterialIndex = DefaultMateriaIndex;
        if (scene->mMeshes[i] == nullptr)
        {
            Logger::Error("ModelLoader", "Mesh pointer is null. Skipping this mesh.");
            continue;
        } 
        else if (scene->mMeshes[i]->mNumVertices == 0 || scene->mMeshes[i]->mNumFaces == 0)
        {
            Logger::Warning("ModelLoader", "Mesh has no vertices or faces. Skipping this mesh.");
            continue;
        }
        else if (scene->mMeshes[i]->mMaterialIndex >= scene->mNumMaterials)
            Logger::Warning("ModelLoader", "Material index is out of range. Using default material.");
        else
            resolvedMaterialIndex = scene->mMeshes[i]->mMaterialIndex;
        
        outModel.meshes.push_back(LoadMesh(scene->mMeshes[i], resolvedMaterialIndex, outModel.skeleton));
        success++;
    }
    return success;
}

MeshData ModelLoader::LoadMesh(const aiMesh* mesh, unsigned int materialIndex, SkeletonData& skeleton)
{
    MeshData meshData = {};

    meshData.materialIndex = materialIndex;
    LoadVertices(mesh, meshData);
    LoadBones(mesh, meshData, skeleton);
    LoadIndices(mesh, meshData);

    return meshData;
}

void ModelLoader::LoadMaterials(const aiScene* scene, ModelData& outModel
, const std::filesystem::path& parentPath)
{
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* material = scene->mMaterials[i];
        MaterialData materialData;
        aiColor4D color;

        if (AI_SUCCESS == aiGetMaterialColor(material,
            AI_MATKEY_COLOR_DIFFUSE, &color))
        {
            materialData.baseColor = { color.r, color.g,
                color.b, color.a };
        }
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString texturePath;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
                materialData.texturePath = (parentPath / texturePath.C_Str()).string();
        }
        else
        {
            std::string msg = parentPath.string() + " Diffuse Texture not found";

            Logger::Info("ModelLoader", msg.c_str());
        }

        outModel.materials.push_back(materialData);
    }
    MaterialData defaultMaterial;
    outModel.materials.push_back(defaultMaterial);
}

bool ModelLoader::AddBoneInfluence(Vertex& vertex, int boneIndex, float weight)
{
    for (size_t i = 0; i < 4; i++)
    {
        if (vertex.boneIndices[i] == -1)
        {
            vertex.boneIndices[i] = boneIndex;
            vertex.boneWeights[i] = weight;
            return true;
        }
    }
    return false;
}

DirectX::XMFLOAT4X4 ModelLoader::ConvertMatrix(const aiMatrix4x4& matrix)
{
    DirectX::XMFLOAT4X4 result;

    result._11 = matrix.a1;
    result._12 = matrix.a2;
    result._13 = matrix.a3;
    result._14 = matrix.a4;

    result._21 = matrix.b1;
    result._22 = matrix.b2;
    result._23 = matrix.b3;
    result._24 = matrix.b4;

    result._31 = matrix.c1;
    result._32 = matrix.c2;
    result._33 = matrix.c3;
    result._34 = matrix.c4;

    result._41 = matrix.d1;
    result._42 = matrix.d2;
    result._43 = matrix.d3;
    result._44 = matrix.d4;

    return result;
}

void ModelLoader::ResolveBoneParents(const aiNode* node, SkeletonData& skeleton, int parentBoneIndex)
{
    const char* name = node->mName.C_Str();

    if (skeleton.nameToIndex.find(name) != skeleton.nameToIndex.end())
    {
        int currentBoneIndex = skeleton.nameToIndex[name];
        skeleton.bones[currentBoneIndex].parentIndex = parentBoneIndex;
        parentBoneIndex = currentBoneIndex;
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        ResolveBoneParents(node->mChildren[i],
            skeleton, parentBoneIndex);
}

void ModelLoader::LoadVertices(const aiMesh* mesh, MeshData& meshData)
{
    for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
    {
        Vertex vertex = {};

        vertex.x = mesh->mVertices[j].x;
        vertex.y = mesh->mVertices[j].y;
        vertex.z = mesh->mVertices[j].z;
        if (mesh->HasTextureCoords(0))
        {
            vertex.u = mesh->mTextureCoords[0][j].x;
            vertex.v = mesh->mTextureCoords[0][j].y;
        }
        if (mesh->HasNormals())
        {
            vertex.nx = mesh->mNormals[j].x;
            vertex.ny = mesh->mNormals[j].y;
            vertex.nz = mesh->mNormals[j].z;
        }
        meshData.vertices.push_back(vertex);
    }
}

void ModelLoader::LoadBones(const aiMesh* mesh, MeshData& meshData, SkeletonData& skeleton)
{
    for (unsigned int i = 0; i < mesh->mNumBones; ++i)
    {
        const aiBone* bone = mesh->mBones[i];
        const char* name = bone->mName.C_Str();
        int boneIndex;

        if (skeleton.nameToIndex.find(name) == skeleton.nameToIndex.end())
        {
            BoneData bonedata = {};
            boneIndex = static_cast<int>(skeleton.bones.size());

            bonedata.name = name;
            bonedata.offsetMatrix = ConvertMatrix(bone->mOffsetMatrix);
            bonedata.parentIndex = -1;

            skeleton.nameToIndex[name] = boneIndex;
            skeleton.bones.push_back(bonedata);
        }
        else
            boneIndex = skeleton.nameToIndex[name];

        for (unsigned int j = 0; j < bone->mNumWeights; ++j)
        {
            unsigned int vertexId = bone->mWeights[j].mVertexId;

            // TODO: 정점 하나가 4개를 초과하는 Bone의 영향을 받을 경우,
            // 가중치가 가장 큰 4개만 남기도록 처리한다.

            // TODO: Vertex Bone Weight 합계가 1이 되도록 정규화한다.

            // TODO: Assimp / DirectX 행렬 규약 및 변환 결과를 검증한다.
            if (vertexId < meshData.vertices.size())
                AddBoneInfluence(meshData.vertices[vertexId],
                    boneIndex, bone->mWeights[j].mWeight);
        }
    }
}

void ModelLoader::LoadIndices(const aiMesh* mesh, MeshData& meshData)
{
    for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
    {
        const aiFace& face = mesh->mFaces[j];

        for (unsigned int k = 0; k < face.mNumIndices; ++k)
            meshData.indices.push_back(face.mIndices[k]);
    }
}