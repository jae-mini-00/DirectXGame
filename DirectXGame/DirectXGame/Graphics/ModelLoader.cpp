#include "Graphics/ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <Windows.h>

bool ModelLoader::Load(std::filesystem::path filePath, ModelData& outModel)
{
    Assimp::Importer importer;
    std::filesystem::path parent_path = filePath.parent_path();

    // TODO: Normal이 없는 모델을 위해 aiProcess_GenSmoothNormals 적용 검토
    const aiScene* scene = importer.ReadFile(
        filePath.string(), aiProcess_Triangulate);

    // TODO: ModelLoader에서 MessageBox 의존성을 제거하고
    //       오류 정보를 상위 계층으로 전달하도록 변경
    if (scene == nullptr)
    {
        MessageBoxA(nullptr, importer.GetErrorString(), 
            "Assimp Error", MB_OK);

        return false;
    }

    outModel.meshes.clear();
    outModel.materials.clear();

    LoadMaterials(scene, outModel, parent_path);
    LoadMeshes(scene, outModel);

    return true;
}




void ModelLoader::LoadMeshes(const aiScene* scene, ModelData& outModel)
{
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        // TODO: 잘못된 Mesh를 continue로 무시하지 말고 로그를 남기도록 변경
        // TODO: materialIndex가 scene->mNumMaterials 범위를 벗어나면
        //       Mesh를 버리지 말고 기본 Material을 사용하도록 변경
        if (scene->mMeshes[i] == nullptr 
            || scene->mMeshes[i]->mNumVertices == 0 || scene->mMeshes[i]->mNumFaces == 0
            || scene->mMeshes[i]->mMaterialIndex >= scene->mNumMaterials)
            continue;

        outModel.meshes.push_back(LoadMesh(scene->mMeshes[i]));
    }
}

MeshData ModelLoader::LoadMesh(const aiMesh* mesh)
{
    MeshData meshData = {};

    meshData.materialIndex = mesh->mMaterialIndex;
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

    for (unsigned int j = 0; j < mesh->mNumFaces; ++j)
    {
        const aiFace& face = mesh->mFaces[j];

        for (unsigned int k = 0; k < face.mNumIndices; ++k)
            meshData.indices.push_back(face.mIndices[k]);
    }

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
        outModel.materials.push_back(materialData);
    }
}