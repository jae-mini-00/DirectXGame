#pragma once

#include <cstdint>

enum class CollisionLayer
{
	Default,
	Layer1,
	Layer2,
	Layer3,
	Layer4,
	Layer5
};

struct CollisionFilter
{
	std::uint32_t collisionLayer = 1u << static_cast<unsigned>(CollisionLayer::Default);
	std::uint32_t collisionMask = 0;
};

constexpr std::uint32_t LayerBit(CollisionLayer layer)
{
	return 1u << static_cast<unsigned>(layer);
}