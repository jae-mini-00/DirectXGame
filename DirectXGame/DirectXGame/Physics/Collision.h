#pragma once

#include "Physics/Collider.h"

namespace Collision
{
    bool IntersectsAABB(const AABBCollider& a, const DirectX::XMFLOAT3& positionA,
        const AABBCollider& b, const DirectX::XMFLOAT3& positionB);
}