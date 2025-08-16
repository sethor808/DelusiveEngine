#pragma once
#include "Agent.h"
#include "DelusiveComponents.h"
#include "DelusiveUtils.h"

class PlayerAgent : public Agent {
public:
	PlayerAgent(const std::string& agentName);
	std::unique_ptr<Agent> Clone() const override;

	void Update(float deltaTime) override;
	void Draw(const glm::mat4& projection) const override;
	void HandleInput(const PlayerInputState&) override;
	void DrawImGui() override;
	std::string GetType() const override;

	void ApplyKnockback(const glm::vec2&, float);
private:
	//std::unique_ptr<Animation> animator;
	glm::vec2 velocity = { 0.0f, 0.0f};
	glm::vec2 impulse = { 0.0f, 0.0f };
	glm::vec2 dodgeDir = { 0.0f, 0.0f };

	float moveSpeed = 400.0f/64.0f;
	float acceleration = 10.0f;
	float deceleration = 8.0f;
	float damping = 0.9f;

	//Dodge variables
	bool dodging = false;
	const float dodgeStrength = moveSpeed * 2.0f; //TESTING ONLY
	const float dodgeDuration = .10f;
	float dodgeTimer = 0.0f;

	float inputLockTimer = 0.0f;
	float dodgeBufferTimer = 0.0f;

	//For input control tracking
	PlayerInputState prevInput;

	//Player Control Functions
	void StartDodge(const glm::vec2&);
	void HandleDodge(float);
	void HandleMovement(float deltaTime);
};