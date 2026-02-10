#pragma once
#include <glm/glm.hpp>

class Collider {
public:
    glm::vec2 position = { 0.0f, 0.0f };  // top-left corner
    glm::vec2 size = { 64.0f, 64.0f };      // width & height
    glm::vec2 offset = { 0.0f, 0.0f };

    bool Intersects(const Collider& other) const;
    glm::vec2 GetPosition();
    glm::vec2 GetOffset();
    glm::vec2& GetMutableOffset();
    void SetOffset(glm::vec2);
    void SetPosition(glm::vec2);
    glm::vec2 GetSize();
    glm::vec2& GetMutableSize();
    void SetSize(glm::vec2);
};