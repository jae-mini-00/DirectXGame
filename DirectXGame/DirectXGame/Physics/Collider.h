#pragma once

#include <DirectXMath.h>

struct AABBCollider
{
	DirectX::XMFLOAT3 center = { 0, 0, 0 };
	DirectX::XMFLOAT3 halfExtents = { 0.5f, 0.5f, 0.5f };
};

struct WorldAABB
{
	DirectX::XMFLOAT3 center = { 0, 0, 0 };
	DirectX::XMFLOAT3 halfExtents = { 0.5f, 0.5f, 0.5f };
};