#include "Graphics/ModelLoader.h"
#include "Core/Logger/Logger.h"
#include "Graphics/Animator.h"

#include <cmath>
#include <DirectXMath.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

LoadResult ModelLoader::Load(std::filesystem::path filePath, ModelData& outModel)
{
    LoadResult lr = {true, LoadError::None, ""};
    Assimp::Importer importer;
    std::filesystem::path parent_path = filePath.parent_path();

    const aiScene* scene = importer.ReadFile(filePath.string(), 
        aiProcess_Triangulate | aiProcess_GenSmoothNormals 
        | aiProcess_PopulateArmatureData | aiProcess_ConvertToLeftHanded);

    if (scene == nullptr)
        return { false, LoadError::FileReadFailed, importer.GetErrorString() };
    
    if (scene->mRootNode == nullptr)
        return { false, LoadError::InvalidScene, "Scene root node is null." };

    outModel.meshes.clear();
    outModel.materials.clear();
    outModel.skeleton.bones.clear();
    outModel.skeleton.nameToIndex.clear();
    outModel.animationClips.clear();
    outModel.nodeHierarchy.nodes.clear();
    outModel.nodeHierarchy.nameToIndex.clear();
    outModel.meshIndexs.clear();

    if (!LoadMaterials(scene, outModel, parent_path))
        return { false, LoadError::InvalidScene, "Failed to load materials." };

    int meshCount = LoadMeshes(scene, outModel);

    if (meshCount < 0)
        return { false, LoadError::InvalidScene, "Failed to load mesh data." };

    if (meshCount == 0)
        return { false, LoadError::NoValidMesh, "Model contains no valid meshes." };
   
    if (!LoadNodes(scene->mRootNode, outModel.nodeHierarchy, -1))
        return { false, LoadError::InvalidScene, "Failed to load node hierarchy." };

    if (!ResolveBoneNodes(outModel.skeleton, outModel.nodeHierarchy))
        return { false, LoadError::InvalidScene, "Failed to resolve bone nodes." };

    if (!ResolveBindPose(scene, outModel.skeleton, outModel.nodeHierarchy))
        return { false, LoadError::InvalidScene, "Failed to resolve bind pose." };

    if (!LoadAnimations(scene, outModel))
        return { false, LoadError::InvalidScene, "Failed to load animations." };

    return lr;
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

    size_t minIndex = 0;

    for (size_t i = 1; i < 4; ++i)
    {
        if (vertex.boneWeights[i] < vertex.boneWeights[minIndex])
            minIndex = i;
    }
    if (weight > vertex.boneWeights[minIndex])
    {
        vertex.boneIndices[minIndex] = boneIndex;
        vertex.boneWeights[minIndex] = weight;
        return true;
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

    DirectX::XMMATRIX matrixDX = DirectX::XMLoadFloat4x4(&result);
    matrixDX = DirectX::XMMatrixTranspose(matrixDX);

    DirectX::XMStoreFloat4x4(&result, matrixDX);

    return result;
}

bool ModelLoader::ResolveBoneNodes(SkeletonData& skeleton, const NodeHierarchyData& hierarchy)
{
    for (const auto& [name, boneIndex] : skeleton.nameToIndex)
    {
        auto it = hierarchy.nameToIndex.find(name);
        if (it != hierarchy.nameToIndex.end())
            skeleton.bones[boneIndex].nodeIndex = it->second;
        else
        {
            std::string msg = name + " node not found";
            Logger::Error("ModelLoader", msg.c_str());
            
            return false;
        }
    }
    return true;
}

aiNode* ModelLoader::FindMeshOwnerNode(aiNode* node, unsigned int meshIndex)
{
    if (node == nullptr)
        return nullptr;

    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        if (node->mMeshes[i] == meshIndex)
            return node;
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        aiNode* findNode = FindMeshOwnerNode(node->mChildren[i], meshIndex);

        if (findNode)
            return findNode;
    }

    return nullptr;
}

aiMatrix4x4 ModelLoader::CalculateNodeGlobalTransform(const aiNode* startNode)
{
    if (startNode == nullptr)
        return aiMatrix4x4();

    const aiNode* parentNode = startNode->mParent;
    aiMatrix4x4 transform = startNode->mTransformation;

    while (parentNode)
    {
        transform = parentNode->mTransformation * transform;
        parentNode = parentNode->mParent;
    }

    return transform;
}


bool ModelLoader::BuildDefaultNodeGlobals(const NodeHierarchyData& hierarchy, std::vector<DirectX::XMFLOAT4X4>& globals)
{
    globals.resize(hierarchy.nodes.size());

    for (size_t i = 0; i < hierarchy.nodes.size(); ++i)
    {
        const NodeData& node = hierarchy.nodes[i];
        DirectX::XMMATRIX global =
            DirectX::XMLoadFloat4x4(&node.localTransform);

        if (node.parentIndex >= 0)
        {
            size_t parent = static_cast<size_t>(node.parentIndex);

            if (parent >= i)
            {
                Logger::Error("ResolveBindPose", "Invalid node parent order");
                return false;
            }
            global *= DirectX::XMLoadFloat4x4(&globals[parent]);
        }
        DirectX::XMStoreFloat4x4(&globals[i], global);
    }

    return true;
}


void ModelLoader::CollectBoneBindGlobals(const aiScene* scene, const SkeletonData& skeleton, std::vector<DirectX::XMFLOAT4X4>& globals)
{
    std::vector<bool> assigned(globals.size(), false);

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        if (mesh == nullptr)
            continue;

        const aiNode* owner = FindMeshOwnerNode(scene->mRootNode, i);
        if (owner == nullptr)
            continue;

        aiMatrix4x4 meshGlobal = CalculateNodeGlobalTransform(owner);

        for (unsigned int j = 0; j < mesh->mNumBones; ++j)
        {
            const aiBone* bone = mesh->mBones[j];
            if (bone == nullptr)
                continue;

            auto it = skeleton.nameToIndex.find(bone->mName.C_Str());
            if (it == skeleton.nameToIndex.end())
                continue;

            int boneIndex = it->second;
            if (boneIndex < 0 ||
                static_cast<size_t>(boneIndex) >= skeleton.bones.size())
                continue;

            int nodeIndex = skeleton.bones[boneIndex].nodeIndex;
            if (nodeIndex < 0 ||
                static_cast<size_t>(nodeIndex) >= globals.size())
                continue;

            size_t index = static_cast<size_t>(nodeIndex);

            if (assigned[index])
            {
                Logger::Warning("ResolveBindPose",
                    "Bone shared by multiple meshes; first binding used");
                continue;
            }

            aiMatrix4x4 inverseOffset = bone->mOffsetMatrix;

            inverseOffset.Inverse();

            aiMatrix4x4 boneBindGlobal = meshGlobal * inverseOffset;

            globals[index] = ConvertMatrix(boneBindGlobal);
            assigned[index] = true;
        }
    }
}

bool ModelLoader::ApplyBindLocalTransforms(NodeHierarchyData& hierarchy, const std::vector<DirectX::XMFLOAT4X4>& globals)
{
    for (size_t i = 0; i < hierarchy.nodes.size(); ++i)
    {
        NodeData& node = hierarchy.nodes[i];
        DirectX::XMMATRIX global = DirectX::XMLoadFloat4x4(&globals[i]);
        DirectX::XMMATRIX local = global;

        if (node.parentIndex >= 0)
        {
            size_t parent = static_cast<size_t>(node.parentIndex);
            DirectX::XMMATRIX parentGlobal = DirectX::XMLoadFloat4x4(&globals[parent]);
            DirectX::XMVECTOR determinant;
            DirectX::XMMATRIX inverseParent = DirectX::XMMatrixInverse(&determinant, parentGlobal);

            if (std::fabs(DirectX::XMVectorGetX(determinant)) < 1e-8f)
            {
                Logger::Warning("ResolveBindPose", "Parent bind transform is not invertible");
                return false;
            }
            local = global * inverseParent;
        }
        DirectX::XMStoreFloat4x4(&node.bindLocalTransform, local);
        node.hasBindPose = true;
    }
    return true;
}

bool ModelLoader::ResolveBindPose(const aiScene* scene, const SkeletonData& skeleton, NodeHierarchyData& hierarchy)
{
    if (scene == nullptr || scene->mRootNode == nullptr)
        return false;

    std::vector<DirectX::XMFLOAT4X4> bindGlobals;

    if (!BuildDefaultNodeGlobals(hierarchy, bindGlobals))
        return false;

    CollectBoneBindGlobals(scene, skeleton, bindGlobals);
    if (!ApplyBindLocalTransforms(hierarchy, bindGlobals))
        return false;

    Logger::Info("ResolveBindPose", "Bind local transforms resolved");
    return true;
}

bool ModelLoader::FillMissingPRS(AnimationChannel& channel, const NodeHierarchyData& hierarchy)
{
    auto it = hierarchy.nameToIndex.find(channel.name);
    if (it == hierarchy.nameToIndex.end())
        return false;

    if (!channel.positions.empty() && !channel.rotations.empty() && !channel.scales.empty())
        return true;
    if (channel.positions.empty() && channel.rotations.empty() && channel.scales.empty())
        return false;

    const NodeData& node = hierarchy.nodes[it->second];
    DirectX::XMVECTOR scale, rotation, position;
    DirectX::XMMATRIX local = DirectX::XMLoadFloat4x4(&node.localTransform);
    bool success = DirectX::XMMatrixDecompose(&scale, &rotation, &position, local);
    if (!success)
        return false;

    if (channel.positions.empty())
    {
        DirectX::XMFLOAT3 value;
        DirectX::XMStoreFloat3(&value, position);
        channel.positions.push_back({ 0.0, value });
    }
    if (channel.rotations.empty())
    {
        DirectX::XMFLOAT4 value;
        DirectX::XMStoreFloat4(&value, rotation);
        channel.rotations.push_back({ 0.0, value });
    }
    if (channel.scales.empty())
    {
        DirectX::XMFLOAT3 value;
        DirectX::XMStoreFloat3(&value, scale);
        channel.scales.push_back({ 0.0, value });
    }

    return true;
}

bool ModelLoader::AreMatricesEqual(const DirectX::XMFLOAT4X4& a, const DirectX::XMFLOAT4X4& b)
{
    for (unsigned int i = 0; i < 4; ++i)
    {
        for (unsigned int j = 0; j < 4; ++j)
        {
            if (std::fabs(a.m[i][j] - b.m[i][j]) > 1e-4f)
                return false;
        }
    }
    return true;
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

bool ModelLoader::LoadBones(const aiMesh* mesh, MeshData& meshData, SkeletonData& skeleton)
{
    for (unsigned int i = 0; i < mesh->mNumBones; ++i)
    {
        const aiBone* bone = mesh->mBones[i];
        if (bone == nullptr)
        {
            Logger::Error("ModelLoader", "Bone pointer is null");
            return false;
        }

        const char* name = bone->mName.C_Str();
        int boneIndex;

        if (skeleton.nameToIndex.find(name) == skeleton.nameToIndex.end())
        {
            BoneData bonedata = {};
            boneIndex = static_cast<int>(skeleton.bones.size());

            bonedata.offsetMatrix = ConvertMatrix(bone->mOffsetMatrix);

            skeleton.nameToIndex[name] = boneIndex;
            skeleton.bones.push_back(bonedata);
        }
        else
        {
            boneIndex = skeleton.nameToIndex[name];
            DirectX::XMFLOAT4X4 offset = ConvertMatrix(bone->mOffsetMatrix);

            if (!AreMatricesEqual(skeleton.bones[boneIndex].offsetMatrix, offset))
            {
                std::string msg = "Bone offset mismatch: " + std::string(name);
                Logger::Error("ModelLoader", msg.c_str());
                return false;
            }
        }

        for (unsigned int j = 0; j < bone->mNumWeights; ++j)
        {
            unsigned int vertexId = bone->mWeights[j].mVertexId;

            if (vertexId < meshData.vertices.size())
                AddBoneInfluence(meshData.vertices[vertexId],
                    boneIndex, bone->mWeights[j].mWeight);
        }
    }

    for (Vertex& vertex : meshData.vertices)
    {
        float totalWeight = 0.0f;

        for (size_t i = 0; i < 4; ++i)
        {
            if (vertex.boneIndices[i] != -1)
                totalWeight += vertex.boneWeights[i];
        }

        if (totalWeight <= 0.0f)
            continue;

        for (size_t i = 0; i < 4; ++i)
        {
            if (vertex.boneIndices[i] != -1)
                vertex.boneWeights[i] /= totalWeight;
        }
    }

    return true;
}

void ModelLoader::LoadIndices(const aiMesh* mesh, MeshData& meshData)
{
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            meshData.indices.push_back(face.mIndices[j]);
    }
}

bool ModelLoader::LoadAnimations(const aiScene* scene, ModelData& outModel)
{
    // TODO: AnimationChannel과 AnimationClip을 vector에 추가할 때
    // 불필요한 데이터 복사를 줄이도록 std::move 적용을 검토한다.

    // TODO: AnimationClip 로딩 중 데이터 오류가 발생했을 때
    // LoadResult를 통해 실패 원인과 메시지를 전달하도록 개선한다.
    for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
    {
        AnimationClip clip = {};
        const aiAnimation* animation = scene->mAnimations[i];
        if (animation == nullptr)
        {
            Logger::Error("ModelLoader", "Animation pointer is null");
            return false;
        }

        clip.name = animation->mName.C_Str();
        clip.duration = animation->mDuration;
        clip.ticksPerSecond = animation->mTicksPerSecond;

        for (unsigned int j = 0; j < animation->mNumChannels; ++j)
        {
            AnimationChannel channel = {};
            const aiNodeAnim* node = animation->mChannels[j];
            if (node == nullptr)
            {
                Logger::Error("ModelLoader", "Animation channel pointer is null");
                return false;
            }

            channel.name = node->mNodeName.C_Str();

            for (unsigned int k = 0; k < node->mNumPositionKeys; ++k)
            {
                PositionKeyframe pframe = {};

                pframe.time = node->mPositionKeys[k].mTime;
                pframe.position.x = node->mPositionKeys[k].mValue.x;
                pframe.position.y = node->mPositionKeys[k].mValue.y;
                pframe.position.z = node->mPositionKeys[k].mValue.z;
                channel.positions.push_back(pframe);
            }

            for (unsigned int k = 0; k < node->mNumRotationKeys; ++k)
            {
                RotationKeyframe rframe = {};

                rframe.time = node->mRotationKeys[k].mTime;
                rframe.rotation.x = node->mRotationKeys[k].mValue.x;
                rframe.rotation.y = node->mRotationKeys[k].mValue.y;
                rframe.rotation.z = node->mRotationKeys[k].mValue.z;
                rframe.rotation.w = node->mRotationKeys[k].mValue.w;
                channel.rotations.push_back(rframe);
            }

            for (unsigned int k = 0; k < node->mNumScalingKeys; ++k)
            {
                ScaleKeyframe sframe = {};

                sframe.time = node->mScalingKeys[k].mTime;
                sframe.scale.x = node->mScalingKeys[k].mValue.x;
                sframe.scale.y = node->mScalingKeys[k].mValue.y;
                sframe.scale.z = node->mScalingKeys[k].mValue.z;
                channel.scales.push_back(sframe);
            }
            if (!FillMissingPRS(channel, outModel.nodeHierarchy))
            {
                Logger::Warning("ModelLoader", "Invalid animation channel");
                
                return false;
            }
            clip.channels.push_back(channel);
        }
        outModel.animationClips.push_back(clip);
    }

    return true;
}

bool ModelLoader::LoadNodes(const aiNode* node, NodeHierarchyData& hierarchy, int parentIndex)
{
    if (node == nullptr)
        return false;

    NodeData nodeData = {};

    nodeData.name = node->mName.C_Str();
    nodeData.parentIndex = parentIndex;
    nodeData.localTransform = ConvertMatrix(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        nodeData.meshIndexs.push_back(node->mMeshes[i]);

    if (hierarchy.nameToIndex.find(nodeData.name) != hierarchy.nameToIndex.end())
    {
        Logger::Error("ModelLoader", "Duplicate node name");
        return false;
    }

    int nodeIndex = static_cast<int>(hierarchy.nodes.size());

    hierarchy.nameToIndex[nodeData.name] = nodeIndex;
    hierarchy.nodes.push_back(nodeData);

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        if (!LoadNodes(node->mChildren[i], hierarchy, nodeIndex))
            return false;
    }

    return true;
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

        MeshData outMesh = {};

        if (!LoadMesh(scene->mMeshes[i], resolvedMaterialIndex, outModel.skeleton, outMesh))
            return -1;

        outModel.meshes.push_back(outMesh);
        outModel.meshIndexs.push_back(i);
        success++;
    }
    return success;
}

bool ModelLoader::LoadMesh(const aiMesh* mesh, unsigned int materialIndex, SkeletonData& skeleton, MeshData& outMesh)
{

    outMesh.materialIndex = materialIndex;
    LoadVertices(mesh, outMesh);

    if (!LoadBones(mesh, outMesh, skeleton))
        return false;

    LoadIndices(mesh, outMesh);

    return true;
}

bool ModelLoader::LoadMaterials(const aiScene* scene, ModelData& outModel, const std::filesystem::path& parentPath)
{
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* material = scene->mMaterials[i];
        if (material == nullptr)
        {
            Logger::Error("ModelLoader", "Material pointer is null");
            return false;
        }

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

    return true;
}