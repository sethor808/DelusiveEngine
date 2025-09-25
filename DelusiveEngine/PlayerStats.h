#pragma once
#include <glm/glm.hpp>
#include "DelusiveMacros.h"

struct PlayerStats {
	float moveSpeed = 400.0f / DELUSIVE_PIXEL_SCALE;
	float acceleration = 10.0f;
	float deceleration = 8.0f;
	float damping = 0.9f;

	float dodgeStrength = moveSpeed * 2.0f; //TESTING ONLY
	float dodgeDuration = .10f;
};