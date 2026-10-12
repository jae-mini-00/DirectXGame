#include "ModelLoader.h"

bool ModelLoader::Load(const char* filePath, ModelData& outModel)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        filePath, aiProcess_Triangulate);

    if (scene == nullptr)
        return false;

    outModel.meshes.clear();

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        MeshData meshData;
        aiMesh* mesh = scene->mMeshes[i];

        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            Vertex vertex = {};
            const aiFace& face = mesh->mFaces[j];

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
            if (mesh->HasVertexColors(0))
            {
                vertex.r = mesh->mColors[0][j].r;
                vertex.g = mesh->mColors[0][j].g;
                vertex.b = mesh->mColors[0][j].b;
                vertex.a = mesh->mColors[0][j].a;
            }
            else
            {
                vertex.r = 1.0f;
                vertex.g = 1.0f;
                vertex.b = 1.0f;
                vertex.a = 1.0f;
            }
            meshData.vertices.push_back(vertex);
            for (unsigned int k = 0; k < face.mNumIndices; ++k)
                meshData.indices.push_back(face.mIndices[k]);
        }

        outModel.meshes.push_back(meshData);
    }

    return true;
}