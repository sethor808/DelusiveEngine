#pragma once
#include "Agent.h"
#include "Animator.h"
#include "Sprite.h"

class PlayerAgent : public Agent {
public:
	PlayerAgent(const std::string& agentName);

	void Update(float deltaTime) override;
	void Draw(const glm::mat4& projection) const override;

private:
	std::unique_ptr<Animator> animator;
	glm::vec2 velocity;
};