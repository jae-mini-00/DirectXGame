#pragma once

struct AnimationClip;

class Animator
{
	public :
		void Play(const AnimationClip* clip, bool loop = true);
		void Update(double deltaTime);

	private :
		const AnimationClip* currentClip;
		double currentTime;
		bool isPlaying;
		bool isLooping;

		Animator();
};