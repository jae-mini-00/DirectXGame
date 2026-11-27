#pragma once

#include "Physics/Collider.h"

namespace Collision
{
    WorldCollider ToWorldCollider(const Collider& collider, const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& scale);
    bool Intersects(const WorldCollider& a, const WorldCollider& b);

    bool IntersectsAABBSphere(const WorldAABB& box, const WorldSphere& sphere);
    bool IntersectsCapsuleSphere(const WorldCapsule& capsule, const WorldSphere& sphere);
    bool IntersectsAABBCapsule(const WorldAABB& box, const WorldCapsule& capsule);

    bool IntersectsAABB(const WorldAABB& a, const WorldAABB& b);
    WorldAABB ToWorldAABB(const AABBCollider& collider, const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& scale);

    bool IntersectsSphere(const WorldSphere& a, const WorldSphere& b);
    WorldSphere ToWorldSphere(const SphereCollider& collider, const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& scale);

    bool IntersectsCapsule(const WorldCapsule& a, const WorldCapsule& b);
    WorldCapsule ToWorldCapsule(const CapsuleCollider& collider, const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& scale);
}