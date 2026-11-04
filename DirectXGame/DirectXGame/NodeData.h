#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <DirectXMath.h>

struct NodeData
{
	std::string name;
	int parentIndex = -1;
	DirectX::XMFLOAT4X4 localTransform;
};

struct NodeHierarchyData
{
	std::vector<NodeData> nodes;
	std::unordered_map<std::string, int> nameToIndex;
};