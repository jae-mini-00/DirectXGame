#include "Scene/Movement.h"
#include "Scene/GameObject.h"

void Movement::Move(Transform& transform, const DirectX::XMFLOAT3 direction, float deltaTime) const
{
	DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
	DirectX::XMVECTOR lengthVector = DirectX::XMVector3Length(dir);
	float length = DirectX::XMVectorGetX(lengthVector);

	if (length <= 0.000001f)
		return;

	dir = DirectX::XMVector3Normalize(dir);
	dir = DirectX::XMVectorScale(dir, speed * deltaTime);
	DirectX::XMStoreFloat3(&transform.position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&transform.position), dir));
}
