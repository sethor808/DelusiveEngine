#pragma once
#include <string>
#include <glm/glm.hpp>

struct SpriteData {
    std::string texturePath;
    glm::vec2 scale = { 1.0f, 1.0f };
    glm::vec2 colliderOffset = { 0.0f, 0.0f };
    glm::vec2 colliderSize = { 1.0f, 1.0f };

    bool LoadFromFile(const std::string& path);
    bool SaveToFile(const std::string& path) const;
};