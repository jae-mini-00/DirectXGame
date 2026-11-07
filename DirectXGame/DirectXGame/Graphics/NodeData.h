#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <DirectXMath.h>

struct NodeData
{
	std::string name;
	int parentIndex = -1;
	std::vector<unsigned int> meshIndexs;
	DirectX::XMFLOAT4X4 localTransform;
	bool hasBindPose = false;
	DirectX::XMFLOAT4X4 bindLocalTransform = {
												1.0f, 0.0f, 0.0f, 0.0f,
												0.0f, 1.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 1.0f, 0.0f,
												0.0f, 0.0f, 0.0f, 1.0f
												};
};

struct NodeHierarchyData
{
	std::vector<NodeData> nodes;
	std::unordered_map<std::string, int> nameToIndex;
};