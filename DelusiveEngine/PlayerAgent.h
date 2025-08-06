#pragma once
#include "Agent.h"
#include "DelusiveComponents.h"

class PlayerAgent : public Agent {
public:
	PlayerAgent(const std::string& agentName);
	std::unique_ptr<Agent> Clone() const override;

	void Update(float deltaTime) override;
	void Draw(const glm::mat4& projection) const override;
	void HandleInput(const PlayerInputState&) override;
	void DrawImGui() override;
private:
	//std::unique_ptr<Animation> animator;
	glm::vec2 velocity = { 0.0f, 0.0f};
	glm::vec2 acceleration = {0.0f, 0.0f};
	glm::vec2 impulse = { 0.0f, 0.0f };

	float moveSpeed = 10.0f;
	float damping = 0.9f;
};