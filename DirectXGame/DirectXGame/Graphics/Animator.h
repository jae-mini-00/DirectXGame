#pragma once

#include "Graphics/AnimationClip.h"

#include <vector>
#include <cstddef>
#include <algorithm>

struct NodeHierarchyData;

class Animator
{
	public :
		Animator();

		void Play(const AnimationClip* clip, bool loop = true);
		void Update(double deltaTime);
		double GetAnimationTime() const;

private :
		const AnimationClip* currentClip;
		double currentTime;
		bool isPlaying;
		bool isLooping;
		std::vector<DirectX::XMFLOAT4X4> globalTransforms;

		template<typename T>
		size_t FindKeyframeIndex(const std::vector<T>& keys, double time) const
		{
			for (size_t i = 1; i < keys.size(); ++i)
			{
				if (keys[i].time > time)
					return i - 1;
			}

			return keys.size() >= 2 ? keys.size() - 2 : 0;
		}
		template<typename T>
		float CalculateKeyframeFactor(const T& start, const T& end, double time) const
		{
			double duration = end.time - start.time;

			if (duration <= 0.0)
				return 0.0f;

			double factor = (time - start.time) / duration;

			return static_cast<float>(std::clamp(factor, 0.0, 1.0));
		}

		DirectX::XMFLOAT3 InterpolatePosition(const std::vector<PositionKeyframe>& positions, double time) const;
		DirectX::XMFLOAT4 InterpolateRotation(const std::vector<RotationKeyframe>& rotations, double time) const;
		DirectX::XMFLOAT3 InterpolateScale(const std::vector<ScaleKeyframe>& scales, double time) const;
		DirectX::XMMATRIX CalculateLocalTransform(const AnimationChannel& channel, double time) const;
		void CalculateGlobalTransforms(const NodeHierarchyData& hierarchy);
};
