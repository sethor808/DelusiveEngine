#pragma once
#include "Agent.h"
#include "EnemyScript.h"
#include "DelusiveComponents.h"

class EnemyAgent : public Agent {
public:
	EnemyAgent(const std::string& agentName);

	//Overrides
	std::unique_ptr<Agent> Clone() const override;
	void Update(float deltaTime) override;
	void Draw(const glm::mat4& projection) const override;
	void OnHit() override;

	//EnemyAgent logic
	void SetScript(std::unique_ptr<EnemyScript>);
	void SetTarget(Agent*);

private:
	Agent* target;
	std::unique_ptr<EnemyScript> logicScript;
	glm::vec2 velocity = { 0.0f, 0.0f };
	glm::vec2 acceleration = { 0.0f, 0.0f };
	glm::vec2 impulse = { 0.0f, 0.0f };

	float moveSpeed = 10.0f;
	float damping = 0.9f;
};