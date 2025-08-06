#pragma once
#include <string>
#include "AnimatorData.h"

class Animation {
public:
    std::string name;
    AnimatorData data;
    

    Animation() = default;
    Animation(AnimatorData _data) : data(_data) {}
    Animation(const std::string& name) : name(name) {}

    void Clear();

    bool SaveToFile(const std::string& path) const;
    bool LoadFromFile(const std::string& path);
};