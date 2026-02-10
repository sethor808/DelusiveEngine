#pragma once
#include <glm/vec2.hpp>

struct PlayerInputState {
    glm::vec2 moveDir{ 0.0f, 0.0f };

    bool lightAttackPressed = false;
    bool heavyAttackPressed = false;
    bool dodgePressed = false;

    bool lightAttackHeld = false;
    bool heavyAttackHeld = false;
};