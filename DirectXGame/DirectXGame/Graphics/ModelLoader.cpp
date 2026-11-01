#include "Graphics/ModelLoader.h"
#include "Core/Logger/Logger.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

LoadResult ModelLoader::Load(std::filesystem::path filePath, ModelData& outModel)
{
    LoadResult lr = {true, LoadError::None, ""};
    Assimp::Importer importer;
    std::filesystem::path parent_path = filePath.parent_path();

    const aiScene* scene = importer.ReadFile(filePath.string(), 
        aiProcess_Triangulate | aiProcess_GenSmoothNormals);

    if (scene == nullptr) {
        lr.success = false;
        lr.error = LoadError::FileReadFailed;
        lr.message = importer.GetErrorString();

        return lr;
    }

    outModel.meshes.clear();
    outModel.materials.clear();

    LoadMaterials(scene, outModel, parent_path);
    if (LoadMeshes(scene, outModel) <= 0) {
        lr.success = false;
        lr.error = LoadError::NoValidMesh;
        lr.message = "Model contains no valid meshes.";
    }

    return lr;
}

int ModelLoader::LoadMeshes(const aiScene* scene, ModelData& outModel)
{
    int success = 0;
    unsigned int DefaultMateriaIndex = static_cast<unsigned int>(outModel.materials.size()) - 1;

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        unsigned int resolvedMaterialIndex = DefaultMateriaIndex;
        if (scene->mMeshes[i] == nullptr) {
            Logger::Error("ModelLoader", "Mesh pointer is null. Skipping this mesh.");
            continue;
        } else if (scene->mMeshes[i]->mNumVertices == 0 || scene->mMeshes[i]->mNumFaces == 0) {
            Logger::Warning("ModelLoader", "Mesh has no vertices or faces. Skipping this mesh.");
            continue;
        }
        else if (scene->mMeshes[i]->mMaterialIndex >= scene->mNumMaterials) {
            Logger::Warning("ModelLoader", "Material index is out of range. Using default material.");
        } else
            resolvedMaterialIndex = scene->mMeshes[i]->mMaterialIndex;
        
        outModel.meshes.push_back(LoadMesh(scene->mMeshes[i], resolvedMaterialIndex));
        success++;
    }
    return success;
}

MeshData ModelLoader::LoadMesh(const aiMesh* mesh, unsigned int materialIndex)
{
    MeshData meshData = {};

    meshData.materialIndex = materialIndex;
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
    MaterialData defaultMaterial;
    outModel.materials.push_back(defaultMaterial);
}