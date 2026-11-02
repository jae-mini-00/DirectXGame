#include "Graphics/Animator.h"
#include "Graphics/AnimationClip.h"

#include <cmath>

Animator::Animator() : currentClip(nullptr), currentTime(0.0), isPlaying(false), isLooping(true) {}

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