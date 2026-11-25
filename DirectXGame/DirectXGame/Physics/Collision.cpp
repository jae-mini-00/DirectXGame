#include "Physics/Collision.h"
#include <cmath>
#include <algorithm>

bool Collision::IntersectsAABBSphere(const WorldAABB& box, const WorldSphere& sphere)
{
    float closestX = std::clamp(sphere.center.x, box.center.x - box.halfExtents.x, box.center.x + box.halfExtents.x);
    float closestY = std::clamp(sphere.center.y, box.center.y - box.halfExtents.y, box.center.y + box.halfExtents.y);
    float closestZ = std::clamp(sphere.center.z, box.center.z - box.halfExtents.z, box.center.z + box.halfExtents.z);
    float dx = sphere.center.x - closestX;
    float dy = sphere.center.y - closestY;
    float dz = sphere.center.z - closestZ;
    float distanceSquared = dx * dx + dy * dy + dz * dz;

    return distanceSquared <= sphere.radius * sphere.radius;
}

bool Collision::IntersectsCapsuleSphere(const WorldCapsule& capsule, const WorldSphere& sphere)
{
    float vectorX = capsule.pointB.x - capsule.pointA.x;
    float vectorY = capsule.pointB.y - capsule.pointA.y;
    float vectorZ = capsule.pointB.z - capsule.pointA.z;
    float dx = sphere.center.x - capsule.pointA.x;
    float dy = sphere.center.y - capsule.pointA.y;
    float dz = sphere.center.z - capsule.pointA.z;
    float lengthSquared = vectorX * vectorX + vectorY * vectorY + vectorZ * vectorZ;
    float dot = dx * vectorX + dy * vectorY + dz * vectorZ;
    float t = lengthSquared > 1e-6f ? std::clamp(dot / lengthSquared, 0.0f, 1.0f) : 0.0f;
    float closestX = capsule.pointA.x + vectorX * t;
    float closestY = capsule.pointA.y + vectorY * t;
    float closestZ = capsule.pointA.z + vectorZ * t;
    float distanceX = sphere.center.x - closestX;
    float distanceY = sphere.center.y - closestY;
    float distanceZ = sphere.center.z - closestZ;
    float distanceSquared = distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ;

    return  distanceSquared <= (capsule.radius + sphere.radius) * (capsule.radius + sphere.radius);
}

bool Collision::IntersectsAABBCapsule(const WorldAABB& box, const WorldCapsule& capsule)
{
    const float start[3] = {capsule.pointA.x, capsule.pointA.y, capsule.pointA.z};
    const float direction[3] = {capsule.pointB.x - capsule.pointA.x, capsule.pointB.y - capsule.pointA.y,
        capsule.pointB.z - capsule.pointA.z};
    const float boxMin[3] = {box.center.x - box.halfExtents.x, box.center.y - box.halfExtents.y,
        box.center.z - box.halfExtents.z};
    const float boxMax[3] = {box.center.x + box.halfExtents.x, box.center.y + box.halfExtents.y,
        box.center.z + box.halfExtents.z};
    float tValues[8] = { 0.0f, 1.0f };
    int tCount = 2;

    for (int axis = 0; axis < 3; ++axis)
    {
        if (direction[axis] == 0.0f)
            continue;

        float tMin = (boxMin[axis] - start[axis]) / direction[axis];
        float tMax = (boxMax[axis] - start[axis]) / direction[axis];

        if (tMin > 0.0f && tMin < 1.0f)
            tValues[tCount++] = tMin;

        if (tMax > 0.0f && tMax < 1.0f)
            tValues[tCount++] = tMax;
    }
    std::sort(tValues, tValues + tCount);

    float minDistanceSquared = 0.0f;

    for (int axis = 0; axis < 3; ++axis)
    {
        float closest = std::clamp(start[axis], boxMin[axis], boxMax[axis]);
        float delta = start[axis] - closest;
        minDistanceSquared += delta * delta;
    }
    for (int i = 1; i < tCount; ++i)
    {
        float t = tValues[i];
        float distanceSquared = 0.0f;

        for (int axis = 0; axis < 3; ++axis)
        {
            float point = start[axis] + direction[axis] * t;
            float closest = std::clamp(point, boxMin[axis], boxMax[axis]);
            float delta = point - closest;

            distanceSquared += delta * delta;
        }
        minDistanceSquared = std::min(minDistanceSquared, distanceSquared);
    }
    for (int i = 0; i < tCount - 1; ++i)
    {
        float lower = tValues[i];
        float upper = tValues[i + 1];

        if (upper <= lower)
            continue;

        float middle = (lower + upper) * 0.5f;
        float A = 0.0f;
        float B = 0.0f;

        for (int axis = 0; axis < 3; ++axis)
        {
            float point = start[axis] + direction[axis] * middle;

            if (point < boxMin[axis])
            {
                A += direction[axis] * direction[axis];
                B += direction[axis] * (start[axis] - boxMin[axis]);
            }
            else if (point > boxMax[axis])
            {
                A += direction[axis] * direction[axis];
                B += direction[axis] * (start[axis] - boxMax[axis]);
            }
        }

        if (A <= 0.0f)
            continue;

        float t = -B / A;

        if (t < lower || t > upper)
            continue;

        float distanceSquared = 0.0f;

        for (int axis = 0; axis < 3; ++axis)
        {
            float point = start[axis] + direction[axis] * t;
            float closest = std::clamp(point, boxMin[axis], boxMax[axis]);
            float delta = point - closest;

            distanceSquared += delta * delta;
        }
        minDistanceSquared = std::min(minDistanceSquared, distanceSquared);
    }

    return minDistanceSquared <= capsule.radius * capsule.radius;
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

bool Collision::IntersectsCapsuleCapsule(const WorldCapsule& a, const WorldCapsule& b)
{
    float dAx = a.pointB.x - a.pointA.x;
    float dBx = b.pointB.x - b.pointA.x;
    float rx = a.pointA.x - b.pointA.x;
    float dAy = a.pointB.y - a.pointA.y;
    float dBy = b.pointB.y - b.pointA.y;
    float ry = a.pointA.y - b.pointA.y;
    float dAz = a.pointB.z - a.pointA.z;
    float dBz = b.pointB.z - b.pointA.z;
    float rz = a.pointA.z - b.pointA.z;
    float lengthSquaredA = dAx * dAx + dAy * dAy + dAz * dAz;
    float lengthSquaredB = dBx * dBx + dBy * dBy + dBz * dBz;
    float dotAB = dAx * dBx + dAy * dBy + dAz * dBz;
    float dotAR = dAx * rx + dAy * ry + dAz * rz;
    float dotBR = dBx * rx + dBy * ry + dBz * rz;
    float s = 0.0f;
    float t = 0.0f;

    constexpr float EPSILON = 1e-6f;

    if (lengthSquaredA <= EPSILON && lengthSquaredB <= EPSILON)
    {
        s = 0.0f;
        t = 0.0f;
    }
    else if (lengthSquaredA <= EPSILON)
    {
        s = 0.0f;
        t = std::clamp(dotBR / lengthSquaredB, 0.0f, 1.0f);
    }
    else if (lengthSquaredB <= EPSILON)
    {
        t = 0.0f;
        s = std::clamp(-dotAR / lengthSquaredA, 0.0f, 1.0f);
    }
    else
    {
        float denominator = lengthSquaredA * lengthSquaredB - dotAB * dotAB;

        if (denominator > EPSILON * lengthSquaredA * lengthSquaredB)
            s = std::clamp((dotAB * dotBR - dotAR * lengthSquaredB) / denominator, 0.0f, 1.0f);
        else
            s = 0.0f;

        t = (dotAB * s + dotBR) / lengthSquaredB;

        if (t < 0.0f)
        {
            t = 0.0f;
            s = std::clamp(-dotAR / lengthSquaredA, 0.0f, 1.0f);
        }
        else if (t > 1.0f)
        {
            t = 1.0f;
            s = std::clamp((dotAB - dotAR) / lengthSquaredA, 0.0f, 1.0f);
        }
    }
    float dx = rx + dAx * s - dBx * t;
    float dy = ry + dAy * s - dBy * t;
    float dz = rz + dAz * s - dBz * t;
    float distanceSquared = dx * dx + dy * dy + dz * dz;
    float radiusSum = a.radius + b.radius;

    return distanceSquared <= radiusSum * radiusSum;
}

WorldCapsule Collision::ToWorldCapsule(const CapsuleCollider& collider, const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& scale)
{
    WorldCapsule worldCapsule;

    worldCapsule.pointA.x = position.x + collider.pointA.x * scale.x;
    worldCapsule.pointB.x = position.x + collider.pointB.x * scale.x;

    worldCapsule.pointA.y = position.y + collider.pointA.y * scale.y;
    worldCapsule.pointB.y = position.y + collider.pointB.y * scale.y;

    worldCapsule.pointA.z = position.z + collider.pointA.z * scale.z;
    worldCapsule.pointB.z = position.z + collider.pointB.z * scale.z;

    worldCapsule.radius = collider.radius * std::max({std::abs(scale.x), std::abs(scale.y), std::abs(scale.z)});

    return worldCapsule;
}