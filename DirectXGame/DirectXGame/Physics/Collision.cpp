#include "Physics/Collision.h"
#include <cmath>

bool Collision::IntersectsAABB(const AABBCollider& a, const DirectX::XMFLOAT3& positionA,
    const AABBCollider& b, const DirectX::XMFLOAT3& positionB)
{
    DirectX::XMFLOAT3 worldCenterA;
    DirectX::XMFLOAT3 worldCenterB;
    
    DirectX::XMStoreFloat3(&worldCenterA, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&positionA), DirectX::XMLoadFloat3(&a.center)));
    DirectX::XMStoreFloat3(&worldCenterB, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&positionB), DirectX::XMLoadFloat3(&b.center)));

    float distanceX = std::abs(worldCenterA.x - worldCenterB.x);
    float distanceY = std::abs(worldCenterA.y - worldCenterB.y);
    float distanceZ = std::abs(worldCenterA.z - worldCenterB.z);

    return ((distanceX <= (a.halfExtents.x + b.halfExtents.x)) && (distanceY <= (a.halfExtents.y + b.halfExtents.y)) && (distanceZ <= (a.halfExtents.z + b.halfExtents.z)));
}