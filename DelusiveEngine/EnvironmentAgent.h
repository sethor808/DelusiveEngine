#pragma once
#include "Agent.h"
#include "DelusiveComponents.h"

class EnvironmentAgent : public Agent {
public:
	EnvironmentAgent(const std::string& agentName);

	//Overrides
	std::unique_ptr<Agent> Clone() const override;
	void Update(float deltaTime) override;
	void Draw(const glm::mat4&) const override;

private:
	glm::vec2 velocity = { 0.0f, 0.0f };
	glm::vec2 acceleration = { 0.0f, 0.0f };
	glm::vec2 impulse = { 0.0f, 0.0f };

	float moveSpeed = 10.0f;
	float damping = 0.9f;
};