#include "Physics/Collision.h"
#include <cmath>

bool Collision::IntersectsAABB(const WorldAABB& a, const WorldAABB& b)
{
    float distanceX = std::abs(a.center.x - b.center.x);
    float distanceY = std::abs(a.center.y - b.center.y);
    float distanceZ = std::abs(a.center.z - b.center.z);

    return ((distanceX <= (a.halfExtents.x + b.halfExtents.x)) && (distanceY <= (a.halfExtents.y + b.halfExtents.y)) && (distanceZ <= (a.halfExtents.z + b.halfExtents.z)));
}

WorldAABB Collision::ToWorldAABB(const AABBCollider& collider, const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& scale)
{
    WorldAABB worldAABB;

    worldAABB.center.x = position.x + collider.center.x * scale.x;
    worldAABB.halfExtents.x = collider.halfExtents.x * std::abs(scale.x);

    worldAABB.center.y = position.y + collider.center.y * scale.y;
    worldAABB.halfExtents.y = collider.halfExtents.y * std::abs(scale.y);

    worldAABB.center.z = position.z + collider.center.z * scale.z;
    worldAABB.halfExtents.z = collider.halfExtents.z * std::abs(scale.z);

    return worldAABB;
}