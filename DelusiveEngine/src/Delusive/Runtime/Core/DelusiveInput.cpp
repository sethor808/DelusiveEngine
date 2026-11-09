#include <Delusive/Runtime/Core/DelusiveInput.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
	constexpr float STICK_DEADZONE = 0.25f;
	constexpr float NAV_THRESHOLD = 0.5f;

	bool Down(uint32_t buttons, SDL_GamepadButton button) {
		return buttons & (1u << button);
	}

	//Snaps a stick to one of four directions for menu steps
	glm::vec2 StickToNav(const glm::vec2& stick) {
		if (std::abs(stick.x) < NAV_THRESHOLD && std::abs(stick.y) < NAV_THRESHOLD) return glm::vec2(0.0f);
		if (std::abs(stick.x) > std::abs(stick.y)) return { stick.x > 0.0f ? 1.0f : -1.0f, 0.0f };
		return { 0.0f, stick.y > 0.0f ? 1.0f : -1.0f };
	}
}

void DelusiveInput::ProcessEvent(const SDL_Event& e) {
	if (e.type == SDL_EVENT_GAMEPAD_ADDED) {
		if (!gamepad) OpenGamepad(e.gdevice.which);
	}
	else if (e.type == SDL_EVENT_GAMEPAD_REMOVED) {
		if (!gamepad || SDL_GetGamepadID(gamepad) != e.gdevice.which) return;

		std::cout << "[DelusiveInput] Gamepad disconnected" << std::endl;
		SDL_CloseGamepad(gamepad);
		gamepad = nullptr;
		OpenAnyGamepad(e.gdevice.which);
	}
}

void DelusiveInput::OpenGamepad(uint32_t id) {
	gamepad = SDL_OpenGamepad(id);
	if (!gamepad) {
		std::cerr << "[DelusiveInput] Failed to open gamepad: " << SDL_GetError() << std::endl;
		return;
	}
	std::cout << "[DelusiveInput] Gamepad connected: " << SDL_GetGamepadName(gamepad) << std::endl;
}

void DelusiveInput::OpenAnyGamepad(uint32_t skip) {
	int count = 0;
	SDL_JoystickID* ids = SDL_GetGamepads(&count);
	for (int i = 0; i < count && !gamepad; ++i) {
		if (ids[i] != skip) OpenGamepad(ids[i]);
	}
	SDL_free(ids);
}

uint32_t DelusiveInput::ReadButtons() const {
	if (!gamepad) return 0;

	uint32_t buttons = 0;
	for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; ++i) {
		if (SDL_GetGamepadButton(gamepad, static_cast<SDL_GamepadButton>(i))) buttons |= 1u << i;
	}
	return buttons;
}

glm::vec2 DelusiveInput::ReadStick() const {
	if (!gamepad) return glm::vec2(0.0f);

	//SDL's stick y points down, the world's points up
	glm::vec2 stick = {
		SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX) / 32767.0f,
		-SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY) / 32767.0f
	};

	//Radial deadzone, rescaled so movement starts from zero at its edge
	float length = glm::length(stick);
	if (length < STICK_DEADZONE) return glm::vec2(0.0f);
	float scaled = std::min((length - STICK_DEADZONE) / (1.0f - STICK_DEADZONE), 1.0f);
	return stick / length * scaled;
}

PlayerInputState DelusiveInput::Poll(uint32_t mouseButtons, bool keyboardFree) {
	PlayerInputState input;
	const bool* keys = SDL_GetKeyboardState(nullptr);
	const uint32_t pad = ReadButtons();
	const uint32_t padBefore = previous.padButtons;
	auto padPressed = [pad, padBefore](SDL_GamepadButton button) {
		return Down(pad, button) && !Down(padBefore, button);
	};

	//Typing in an editor field must not toggle game UI
	input.keys = keyboardFree ? keys : nullptr;
	input.padButtons = pad;

	//Keys win over the stick so the two never fight
	glm::vec2 keyMove = {
		static_cast<float>(keys[SDL_SCANCODE_D]) - keys[SDL_SCANCODE_A],
		static_cast<float>(keys[SDL_SCANCODE_W]) - keys[SDL_SCANCODE_S]
	};
	input.moveDir = glm::length(keyMove) > 0.0f ? glm::normalize(keyMove) : ReadStick();

	input.lightAttackHeld = keys[SDL_SCANCODE_J] || (mouseButtons & SDL_BUTTON_LMASK) || Down(pad, SDL_GAMEPAD_BUTTON_WEST);
	input.heavyAttackHeld = keys[SDL_SCANCODE_K] || (mouseButtons & SDL_BUTTON_RMASK) || Down(pad, SDL_GAMEPAD_BUTTON_NORTH);
	input.lightAttackPressed = input.lightAttackHeld && !previous.lightAttackHeld;
	input.heavyAttackPressed = input.heavyAttackHeld && !previous.heavyAttackHeld;
	input.dodgeHeld = keys[SDL_SCANCODE_LSHIFT] || Down(pad, SDL_GAMEPAD_BUTTON_SOUTH);
	input.dodgePressed = input.dodgeHeld && !previous.dodgeHeld;

	if (padPressed(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) input.swapDir = 1;
	else if (padPressed(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)) input.swapDir = -1;

	glm::vec2 nav = {
		static_cast<float>(Down(pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) - Down(pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT),
		static_cast<float>(Down(pad, SDL_GAMEPAD_BUTTON_DPAD_UP)) - Down(pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN)
	};
	if (nav == glm::vec2(0.0f)) nav = StickToNav(ReadStick());
	if (nav != previousNav) input.navDir = nav;
	previousNav = nav;

	input.confirmPressed = padPressed(SDL_GAMEPAD_BUTTON_SOUTH);
	input.backPressed = padPressed(SDL_GAMEPAD_BUTTON_EAST);

	previous = input;
	return input;
}
