#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <DirectXMath.h>

struct BoneData
{
    std::string name;
    int parentIndex = -1;
    DirectX::XMFLOAT4X4 offsetMatrix;
};

struct SkeletonData
{
    std::vector<BoneData> bones;
    std::unordered_map<std::string, int> nameToIndex;
};

class Skeleton
{
    private:
        std::vector<BoneData> bones;
        std::unordered_map<std::string, int> boneNameToIndex;
};