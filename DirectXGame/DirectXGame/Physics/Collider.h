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

struct SphereCollider
{
	DirectX::XMFLOAT3 center = { 0.0f, 0.0f, 0.0f };
	float radius = 0.5f;
};

struct WorldSphere
{
	DirectX::XMFLOAT3 center = { 0.0f, 0.0f, 0.0f };
	float radius = 0.5f;
};