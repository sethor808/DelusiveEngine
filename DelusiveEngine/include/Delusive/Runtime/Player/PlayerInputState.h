#pragma once
#include <glm/vec2.hpp>
#include <cstdint>

//One frame of player intent from keyboard, mouse and gamepad together (see DelusiveInput).
//Pressed fields are true only on the frame the input went down, Held fields while it stays down
struct PlayerInputState {
	glm::vec2 moveDir{ 0.0f, 0.0f };

	bool lightAttackPressed = false;
	bool heavyAttackPressed = false;
	bool dodgePressed = false;
	//+1 next weapon, -1 previous, 0 none
	int swapDir = 0;

	bool lightAttackHeld = false;
	bool heavyAttackHeld = false;
	bool dodgeHeld = false;

	//Menu navigation: one step per press
	glm::vec2 navDir{ 0.0f, 0.0f };
	bool confirmPressed = false;
	bool backPressed = false;

	//Raw SDL keyboard state indexed by scancode, for bindings like UI state keys
	const bool* keys = nullptr;
	//Gamepad buttons held, one bit per SDL_GamepadButton
	uint32_t padButtons = 0;
};
