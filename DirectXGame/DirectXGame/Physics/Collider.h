#pragma once

#include <DirectXMath.h>
#include <variant>

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

struct CapsuleCollider
{
	DirectX::XMFLOAT3 pointA = { 0.0f, -0.5f, 0.0f };
	DirectX::XMFLOAT3 pointB = { 0.0f, 0.5f, 0.0f };
	float radius = 0.5f;
};

struct WorldCapsule
{
	DirectX::XMFLOAT3 pointA = { 0.0f, -0.5f, 0.0f };
	DirectX::XMFLOAT3 pointB = { 0.0f, 0.5f, 0.0f };
	float radius = 0.5f;
};

using Collider = std::variant<std::monostate, AABBCollider,
	SphereCollider, CapsuleCollider>;

using WorldCollider = std::variant<std::monostate, WorldAABB,
	WorldSphere, WorldCapsule>;