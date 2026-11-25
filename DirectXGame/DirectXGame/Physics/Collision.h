#pragma once

#include "Physics/Collider.h"

namespace Collision
{
    bool IntersectsAABB(const WorldAABB& a, const WorldAABB& b);
    WorldAABB ToWorldAABB(const AABBCollider& collider, const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& scale);
    bool IntersectsSphere(const WorldSphere& a, const WorldSphere& b);
    WorldSphere ToWorldSphere(const SphereCollider& collider, const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& scale);
}