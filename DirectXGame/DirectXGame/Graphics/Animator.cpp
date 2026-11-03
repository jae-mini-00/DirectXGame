#include "Graphics/Animator.h"
#include "Core/Logger/Logger.h"

#include <cmath>
#include <DirectXMath.h>

Animator::Animator() : currentClip(nullptr), currentTime(0.0), isPlaying(false), isLooping(true) {}
double Animator::GetAnimationTime() const { return currentTime; }

void Animator::Play(const AnimationClip* clip, bool loop)
{
	if (clip == nullptr)
		return;

	currentClip = clip;
	currentTime = 0.0;
	isPlaying = true;
	isLooping = loop;
}

void Animator::Update(double deltaTime)
{
	if (currentClip == nullptr || !isPlaying || deltaTime <= 0.0)
		return;

	double duration = currentClip->duration;

	if (duration <= 0.0)
	{
		currentTime = 0.0;
		isPlaying = false;
		return;
	}

	double ticksPerSecond = currentClip->ticksPerSecond > 0.0 ? currentClip->ticksPerSecond : 25.0;

	currentTime += deltaTime * ticksPerSecond;
	if (currentTime >= duration)
	{
		if (isLooping)
			currentTime = std::fmod(currentTime, duration);
		else
		{
			currentTime = duration;
			isPlaying = false;
		}
	}
}

DirectX::XMFLOAT3 Animator::InterpolatePosition(const std::vector<PositionKeyframe>& positions, double time) const
{
	DirectX::XMFLOAT3 position = {};

	if (positions.empty() || positions.size() == 1)
		return positions.empty() ? position : positions[0].position;

	size_t index = FindKeyframeIndex(positions, time);
	float interpolate = CalculateKeyframeFactor(positions[index], positions[index + 1], time);
	DirectX::XMVECTOR start = DirectX::XMLoadFloat3(&positions[index].position);
	DirectX::XMVECTOR end = DirectX::XMLoadFloat3(&positions[index + 1].position);
	DirectX::XMStoreFloat3(&position, DirectX::XMVectorLerp(start, end, interpolate));

	return position;
}

DirectX::XMFLOAT4 Animator::InterpolateRotation(const std::vector<RotationKeyframe>& rotations, double time) const
{
	DirectX::XMFLOAT4 rotation = {0, 0, 0, 1};

	if (rotations.empty() || rotations.size() == 1)
		return rotations.empty() ? rotation : rotations[0].rotation;

	size_t index = FindKeyframeIndex(rotations, time);
	float interpolate = CalculateKeyframeFactor(rotations[index], rotations[index + 1], time);
	DirectX::XMVECTOR start = DirectX::XMLoadFloat4(&rotations[index].rotation);
	DirectX::XMVECTOR end = DirectX::XMLoadFloat4(&rotations[index + 1].rotation);
	DirectX::XMStoreFloat4(&rotation, DirectX::XMQuaternionSlerp(start, end, interpolate));

	return rotation;
}

DirectX::XMFLOAT3 Animator::InterpolateScale(const std::vector<ScaleKeyframe>& scales, double time) const
{
	DirectX::XMFLOAT3 scale = {1, 1, 1};

	if (scales.empty() || scales.size() == 1)
		return scales.empty() ? scale : scales[0].scale;

	size_t index = FindKeyframeIndex(scales, time);
	float interpolate = CalculateKeyframeFactor(scales[index], scales[index + 1], time);
	DirectX::XMVECTOR start = DirectX::XMLoadFloat3(&scales[index].scale);
	DirectX::XMVECTOR end = DirectX::XMLoadFloat3(&scales[index + 1].scale);
	DirectX::XMStoreFloat3(&scale, DirectX::XMVectorLerp(start, end, interpolate));

	return scale;
}

DirectX::XMMATRIX Animator::CalculateLocalTransform(
	const AnimationChannel& channel, double time) const
{
	DirectX::XMFLOAT3 position = InterpolatePosition(channel.positions, time);
	DirectX::XMFLOAT4 rotation = InterpolateRotation(channel.rotations, time);
	DirectX::XMFLOAT3 scale = InterpolateScale(channel.scales, time);

	DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&scale));
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&position));

	return S * R * T;
}
