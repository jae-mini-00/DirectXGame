#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <DirectXMath.h>

struct BoneData
{
    int nodeIndex = -1;
    DirectX::XMFLOAT4X4 offsetMatrix;
};

struct SkeletonData
{
    std::vector<BoneData> bones;
    std::unordered_map<std::string, int> nameToIndex;
};