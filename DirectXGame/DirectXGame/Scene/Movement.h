#pragma once

#include <DirectXMath.h>

struct Transform;

class Movement
{
	public:
		float GetSpeed() const;
		void SetSpeed(float speed);
		void Move(Transform& transform, const DirectX::XMFLOAT3 direction, float deltaTime) const;
	private:
		float speed = 5.0f;
};

