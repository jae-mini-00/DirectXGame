#include "ModelLoader.h"

bool ModelLoader::Load(const char* filePath, ModelData& outModel)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        filePath, aiProcess_Triangulate);

    if (scene == nullptr)
    {
        MessageBoxA(nullptr, importer.GetErrorString(), 
            "Assimp Error", MB_OK);

        return false;
    }

    outModel.meshes.clear();
    outModel.materials.clear();

    LoadMaterials(scene, outModel);
    LoadMeshes(scene, outModel);

    return true;
}


/*
* continue 부분 후에 로그 형식으로 변경
* MaterialIndex >= scene->mNumMaterials 는 기본 머티리얼로 대체로 변경
*/
void ModelLoader::LoadMeshes(const aiScene* scene, ModelData& outModel)
{
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
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

#include <string>

void ModelLoader::LoadMaterials(const aiScene* scene, ModelData& outModel)
{
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* material = scene->mMaterials[i];
        MaterialData materialData;
        aiColor4D color;

        if (AI_SUCCESS == aiGetMaterialColor(material,
            AI_MATKEY_BASE_COLOR, &color))
        {
            materialData.baseColor = { color.r, color.g,
                color.b, color.a };
        }
        if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString texturePath;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
                materialData.texturePath = texturePath.C_Str();

        }
        outModel.materials.push_back(materialData);
    }
}