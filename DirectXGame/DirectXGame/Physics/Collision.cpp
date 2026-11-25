#include "Physics/Collision.h"
#include <cmath>
#include <algorithm>

bool Collision::IntersectsAABBSphere(const WorldAABB& box, const WorldSphere& sphere)
{
    float min = box.center.x - box.halfExtents.x;
    float max = box.center.x + box.halfExtents.x;
    float closestX = std::clamp(sphere.center.x, min, max);
    min = box.center.y - box.halfExtents.y;
    max = box.center.y + box.halfExtents.y;
    float closestY = std::clamp(sphere.center.y, min, max);
    min = box.center.z - box.halfExtents.z;
    max = box.center.z + box.halfExtents.z;
    float closestZ = std::clamp(sphere.center.z, min, max);
    float dx = sphere.center.x - closestX;
    float dy = sphere.center.y - closestY;
    float dz = sphere.center.z - closestZ;
    float distanceSquared = dx * dx + dy * dy + dz * dz;

    return distanceSquared <= sphere.radius * sphere.radius;
}

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

bool Collision::IntersectsSphere(const WorldSphere& a, const WorldSphere& b)
{
    float dx = a.center.x - b.center.x;
    float dy = a.center.y - b.center.y;
    float dz = a.center.z - b.center.z;
    float distanceSquared = dx * dx + dy * dy + dz * dz;
    float radiusSum = a.radius + b.radius;

    return distanceSquared <= radiusSum * radiusSum;
}

WorldSphere Collision::ToWorldSphere(const SphereCollider& collider, const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& scale)
{
    WorldSphere worldSphere;

    worldSphere.center.x = position.x + collider.center.x * scale.x;
    worldSphere.center.y = position.y + collider.center.y * scale.y;
    worldSphere.center.z = position.z + collider.center.z * scale.z;
    worldSphere.radius = collider.radius * std::max({std::abs(scale.x), std::abs(scale.y), std::abs(scale.z)});

    return worldSphere;
}