#pragma once
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Components/DelusiveComponents.h>

class EnvironmentAgent : public Agent {
public:
	EnvironmentAgent(DelusiveInstance&);
	EnvironmentAgent() = delete;

	//Overrides
	void Update(float deltaTime) override;
	void Draw(const glm::mat4&) const override;
	std::string GetType() const override;
	bool IsStatic() const override { return true; }

private:
	glm::vec2 velocity = { 0.0f, 0.0f };
	glm::vec2 acceleration = { 0.0f, 0.0f };
	glm::vec2 impulse = { 0.0f, 0.0f };

	float moveSpeed = 10.0f;
	float damping = 0.9f;
};