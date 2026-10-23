#pragma once
#include <glm/vec2.hpp>

struct PlayerInputState {
    glm::vec2 moveDir{ 0.0f, 0.0f };

    bool lightAttackPressed = false;
    bool heavyAttackPressed = false;
    bool dodgePressed = false;
    //+1 next weapon, -1 previous, 0 none
    int swapDir = 0;

    bool lightAttackHeld = false;
    bool heavyAttackHeld = false;
};