#pragma once
#include "Agent.h"
#include "DelusiveComponents.h"
#include "DelusiveUtils.h"
#include "PlayerStats.h"

class Talisman;

class PlayerAgent : public Agent {
public:
	PlayerAgent(DelusiveRenderer&);
	PlayerAgent() = delete;
	std::unique_ptr<Agent> Clone(Scene*) const override;

	void Update(float deltaTime) override;
	void Draw(const glm::mat4& projection) const override;
	void HandleInput(const PlayerInputState&) override;
	std::string GetType() const override;

	void ApplyKnockback(const glm::vec2&, float);

	void TakeDamage() override;
	const std::vector<std::unique_ptr<Talisman>>& GetTalismans() { return talismans; }

	void TriggerInvul(float);
	void ResetStats();

private:
	PlayerStats base;
	PlayerStats modified;

	//HP Management
	std::vector<std::unique_ptr<Talisman>> talismans;

	glm::vec2 velocity = { 0.0f, 0.0f };
	glm::vec2 impulse = { 0.0f, 0.0f };
	glm::vec2 dodgeDir = { 0.0f, 0.0f };

	//Dodge variables
	bool dodging = false;

	float dodgeTimer = 0.0f;

	float inputLockTimer = 0.0f;
	float dodgeBufferTimer = 0.0f;
	float invulTimer = 0.0f;

	//For input control tracking
	PlayerInputState prevInput;

	//PlayerState Helpers
	bool CheckIfDead();

	//Player Control Functions
	void StartDodge(const glm::vec2&);
	void HandleDodge(float);
	void HandleMovement(float deltaTime);
};