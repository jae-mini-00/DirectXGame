#include "Logger.h"
#include "Graphics/AnimationClip.h"
#include "Graphics/SkeletonData.h"

#include <string>
#include <Windows.h>

void Logger::Info(const char* where, const char* message)
{
	OutputDebugStringA("[Info] ");
	OutputDebugStringA(where);
	OutputDebugStringA(": ");
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
}

void Logger::Warning(const char* where, const char* message)
{
	OutputDebugStringA("[Warning] ");
	OutputDebugStringA(where);
	OutputDebugStringA(": ");
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
}

void Logger::Error(const char* where, const char* message)
{
	OutputDebugStringA("[Error] ");
	OutputDebugStringA(where);
	OutputDebugStringA(": ");
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
}

void Logger::LogAnimationClip(const AnimationClip& clip)
{
    const char* where = "AnimationClip";

    Info(where, "========================================");
    Info(where, ("Name           : " + clip.name).c_str());
    Info(where, ("Duration       : " + std::to_string(clip.duration)).c_str());
    Info(where, ("TicksPerSecond : " + std::to_string(clip.ticksPerSecond)).c_str());
    Info(where, ("Channel Count  : " + std::to_string(clip.channels.size())).c_str());
    Info(where, "----------------------------------------");

    for (size_t i = 0; i < clip.channels.size(); ++i)
    {
        const AnimationChannel& channel = clip.channels[i];

        Info(where, ("Channel[" + std::to_string(i) + "]").c_str());
        Info(where, ("  Name          : " + channel.name).c_str());
        Info(where, ("  Position Keys : " + std::to_string(channel.positions.size())).c_str());
        Info(where, ("  Rotation Keys : " + std::to_string(channel.rotations.size())).c_str());
        Info(where, ("  Scale Keys    : " + std::to_string(channel.scales.size())).c_str());

        Info(where, "");
    }

    Info(where, "========================================");
}

void Logger::LogAnimationChannel(const AnimationChannel& channel)
{
    const char* where = "AnimationChannel";

    Info(where, "========================================");
    Info(where, ("Name          : " + channel.name).c_str());
    Info(where, ("Position Keys : " + std::to_string(channel.positions.size())).c_str());
    Info(where, ("Rotation Keys : " + std::to_string(channel.rotations.size())).c_str());
    Info(where, ("Scale Keys    : " + std::to_string(channel.scales.size())).c_str());

    Info(where, "----------------------------------------");
    Info(where, "[Position Keyframes]");

    for (size_t i = 0; i < channel.positions.size(); ++i)
    {
        const PositionKeyframe& key = channel.positions[i];

        std::string msg =
            "  Key[" + std::to_string(i) + "]" +
            " Time: " + std::to_string(key.time) +
            " XYZ: (" +
            std::to_string(key.position.x) + ", " +
            std::to_string(key.position.y) + ", " +
            std::to_string(key.position.z) + ")";

        Info(where, msg.c_str());
    }

    Info(where, "----------------------------------------");
    Info(where, "[Rotation Keyframes]");

    for (size_t i = 0; i < channel.rotations.size(); ++i)
    {
        const RotationKeyframe& key = channel.rotations[i];

        std::string msg =
            "  Key[" + std::to_string(i) + "]" +
            " Time: " + std::to_string(key.time) +
            " XYZW: (" +
            std::to_string(key.rotation.x) + ", " +
            std::to_string(key.rotation.y) + ", " +
            std::to_string(key.rotation.z) + ", " +
            std::to_string(key.rotation.w) + ")";

        Info(where, msg.c_str());
    }

    Info(where, "----------------------------------------");
    Info(where, "[Scale Keyframes]");

    for (size_t i = 0; i < channel.scales.size(); ++i)
    {
        const ScaleKeyframe& key = channel.scales[i];

        std::string msg =
            "  Key[" + std::to_string(i) + "]" +
            " Time: " + std::to_string(key.time) +
            " XYZ: (" +
            std::to_string(key.scale.x) + ", " +
            std::to_string(key.scale.y) + ", " +
            std::to_string(key.scale.z) + ")";

        Info(where, msg.c_str());
    }

    Info(where, "========================================");
}

void Logger::LogSkeleton(const SkeletonData& skeleton)
{
    const char* where = "Skeleton";

    Info(where, "========================================");
    Info(where, ("Bone Count : " +
        std::to_string(skeleton.bones.size())).c_str());
    Info(where, "----------------------------------------");

    for (size_t i = 0; i < skeleton.bones.size(); ++i)
    {
        LogBone(skeleton.bones[i], i);
    }

    Info(where, "========================================");
}

void Logger::LogBone(const BoneData& bone, size_t index)
{
    const char* where = "Skeleton";

    Info(where, ("Bone[" + std::to_string(index) + "]").c_str());
    Info(where, ("  Node Index : " +
        std::to_string(bone.nodeIndex)).c_str());

    Info(where, "");
}