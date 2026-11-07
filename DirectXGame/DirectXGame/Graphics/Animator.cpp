#include "Graphics/Animator.h"
#include "Core/Logger/Logger.h"
#include "Graphics/NodeData.h"
#include "Graphics/SkeletonData.h"

#include <cmath>
#include <DirectXMath.h>

Animator::Animator() : currentClip(nullptr), currentTime(0.0), isPlaying(false), isLooping(true) {}
double Animator::GetAnimationTime() const { return currentTime; }
const std::vector<DirectX::XMFLOAT4X4>& Animator::GetSkinningMatrices() const { return skinningMatrices; }

const DirectX::XMFLOAT4X4* Animator::GetNodeGlobalTransform(int nodeIndex) const
{ 
	if (nodeIndex < 0 || static_cast<size_t>(nodeIndex) >= globalTransforms.size())
		return nullptr;

	return &globalTransforms[nodeIndex];
}


void Animator::Play(const AnimationClip* clip, bool loop)
{
	if (clip == nullptr)
		return;

	currentClip = clip;
	currentTime = 0.0;
	isPlaying = true;
	isLooping = loop;
}

void Animator::Update(double deltaTime, const NodeHierarchyData& hierarchy, const SkeletonData& skeleton)
{
	if (currentClip != nullptr && isPlaying && deltaTime > 0.0)
	{
		double duration = currentClip->duration;

		if (duration <= 0.0)
		{
			currentTime = 0.0;
			isPlaying = false;
		}
		else
		{
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
	}
	CalculateGlobalTransforms(hierarchy);
	CalculateSkinningMatrices(skeleton);
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

void Animator::CalculateGlobalTransforms(const NodeHierarchyData& hierarchy)
{
	globalTransforms.resize(hierarchy.nodes.size());

	for (size_t i = 0; i < hierarchy.nodes.size(); ++i)
	{
		const NodeData& node = hierarchy.nodes[i];
		const DirectX::XMFLOAT4X4& local = (currentClip == nullptr && node.hasBindPose)
			? node.bindLocalTransform : node.localTransform;
		DirectX::XMMATRIX transform = DirectX::XMLoadFloat4x4(&local);

		if (currentClip != nullptr)
		{
			for (const AnimationChannel& channel : currentClip->channels)
			{
				if (channel.name == node.name)
				{
					transform = CalculateLocalTransform(channel, currentTime);
					break;
				}
			}
		}
		if (node.parentIndex != -1)
			transform = DirectX::XMMatrixMultiply(transform,
				DirectX::XMLoadFloat4x4(&globalTransforms[node.parentIndex]));

		DirectX::XMStoreFloat4x4(&globalTransforms[i], transform);
	}
}

void Animator::CalculateSkinningMatrices(const SkeletonData& skeleton)
{
	skinningMatrices.resize(skeleton.bones.size());

	for (unsigned int i = 0; i < skeleton.bones.size(); ++i)
	{
		const BoneData& bone = skeleton.bones[i];

		if (bone.nodeIndex < 0 || static_cast<size_t>(bone.nodeIndex) >= globalTransforms.size())
			continue;

		DirectX::XMFLOAT4X4 skining = {};
		DirectX::XMMATRIX  offsetMatrix = DirectX::XMLoadFloat4x4(&bone.offsetMatrix);
		DirectX::XMMATRIX transform = DirectX::XMLoadFloat4x4(&globalTransforms[bone.nodeIndex]);
		DirectX::XMMATRIX result = DirectX::XMMatrixMultiply(offsetMatrix, transform);

		DirectX::XMStoreFloat4x4(&skining, result);
		skinningMatrices[i] = skining;
	}
}