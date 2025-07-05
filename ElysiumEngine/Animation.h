#pragma once
#include <string>
#include "AnimatorData.h"

class Animation {
public:
    std::string name;
    AnimatorData data;
    

    Animation() = default;
    Animation(const std::string& name) : name(name) {}

    bool SaveToFile(const std::string& path) const;
    bool LoadFromFile(const std::string& path);
};