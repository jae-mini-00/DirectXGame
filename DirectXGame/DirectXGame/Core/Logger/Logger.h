#pragma once

#include <vector>

struct AnimationClip;
struct AnimationChannel;
struct SkeletonData;
struct BoneData;

class Logger
{
	public :
		static void Info(const char* where, const char* message);
		static void Warning(const char* where, const char* message);
		static void Error(const char* where, const char* message);

		static void LogAnimationClip(const AnimationClip& clip);
		static void LogAnimationChannel(const AnimationChannel& channel);

		static void LogSkeleton(const SkeletonData& skeleton);
		static void LogBone(const BoneData& bone, size_t index);
};