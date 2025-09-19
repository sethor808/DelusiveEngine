#pragma once
#include "Agent.h"
#include <SDL3/SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

class CameraAgent : public Agent {
public:
	CameraAgent();
	CameraAgent(const std::string&);

	void Update(float) override;
	void Draw(const glm::mat4&) const override {}
	void DrawImGui() override;
	std::unique_ptr<Agent> Clone(Scene*) const override;
	std::string GetType() const override;

	void HandleInput(const glm::vec2&, bool, float);
	void Follow(Agent*);
	void SetZoom(float);
	float GetZoom() const;

	glm::mat4 GetViewProjection(int, int) const;
	glm::mat4 GetViewProjectionFromWindow(SDL_Window* window) const;
private:
	float zoom = 1.0f;
	glm::vec2 panOffset = glm::vec2(0.0f);
	Agent* followTarget = nullptr;

	bool dragging = false;
	glm::vec2 lastMousePos = glm::vec2(0.0f);
};