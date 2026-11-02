#pragma once

#include <string>
#include <vector>
#include <DirectXMath.h>

struct PositionKeyframe
{
    double time;
    DirectX::XMFLOAT3 position;
};

struct RotationKeyframe
{
    double time;
    DirectX::XMFLOAT4 rotation;
};

struct ScaleKeyframe
{
    double time;
    DirectX::XMFLOAT3 scale;
};

struct AnimationChannel
{
    std::string name;
    std::vector<PositionKeyframe> positions;
    std::vector<RotationKeyframe> rotations;
    std::vector<ScaleKeyframe> scales;
};

struct AnimationClip
{
    std::string name;
    double duration;
    double ticksPerSecond;
    std::vector<AnimationChannel> channels;
};