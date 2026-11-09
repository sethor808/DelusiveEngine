#pragma once
#include <Delusive/Runtime/Player/PlayerInputState.h>

union SDL_Event;
struct SDL_Gamepad;

//Builds each frame's PlayerInputState from keyboard, mouse and one gamepad at once, so
//neither device needs switching on. The first connected gamepad is used; hot-plug is handled.
class DelusiveInput {
public:
	//Every polled event goes through here for gamepad connects and disconnects
	void ProcessEvent(const SDL_Event&);
	//mouseButtons: SDL mouse button mask, 0 while the mouse is over an editor panel
	//keyboardFree: no editor field is being typed in
	PlayerInputState Poll(uint32_t mouseButtons, bool keyboardFree);
private:
	//SDL_Quit closes it, so there is no destructor
	SDL_Gamepad* gamepad = nullptr;
	PlayerInputState previous;
	glm::vec2 previousNav{ 0.0f, 0.0f };

	void OpenGamepad(uint32_t);
	void OpenAnyGamepad(uint32_t skip);
	uint32_t ReadButtons() const;
	glm::vec2 ReadStick() const;
};
