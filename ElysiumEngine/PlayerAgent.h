#pragma once
#include "Agent.h"
#include "ElysiumComponents.h"
#include "Sprite.h"

class PlayerAgent : public Agent {
public:
	PlayerAgent(const std::string& agentName);
	std::unique_ptr<Agent> Clone() const override;

	void Update(float deltaTime) override;
	void Draw(const glm::mat4& projection) const override;

private:
	//std::unique_ptr<Animation> animator;
	glm::vec2 velocity;
};