#pragma once

#include <string>

enum class LoadError
{
    None,
    FileReadFailed,
    InvalidScene,
    NoValidMesh,
    MaterialError,
    Unknown
};

struct LoadResult
{
    bool success;
    LoadError error;
    std::string message;
};