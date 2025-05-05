#pragma once
#include "Sprite.h"
#include <SDL3/SDL.h>
#include <algorithm>

enum class ColliderAction {
	None,
	Drag,
	ResizeLeft,
	ResizeRight,
	ResizeTop,
	ResizeBottom
};

class SpriteEditor {
public:
	void DrawUI(Sprite& sprite);
	void HandleMouse(SDL_Window* window, Sprite& sprite, const glm::mat4& projection);

private:
	bool isDragging = false;
	glm::vec2 dragOffset = { 0.0f, 0.0f };
	float resizeThreshhold = 8.0f;
	ColliderAction currentAction = ColliderAction::None;
	float resizeThreshold = 8.0f;
	int selectedColliderIndex = -1;
};