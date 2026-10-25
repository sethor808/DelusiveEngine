#pragma once
#include <glm/glm.hpp>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>

struct PlayerStats {
	float moveSpeed = 400.0f / DELUSIVE_PIXEL_SCALE;
	float acceleration = 10.0f;
	float deceleration = 8.0f;
	float damping = 0.9f;

	//Dodge speed is a multiple of moveSpeed, so speed boosts also dodge further
	float dodgeSpeedScale = 2.0f;
	float dodgeDuration = .10f;

	//Combat
	float invulDuration = 0.0f; //Granted after every hit taken
	float hitboxScale = 1.0f;
	float damageMultiplier = 1.0f;
	float statusStackMultiplier = 1.0f; //TODO: Read once status effects exist
};