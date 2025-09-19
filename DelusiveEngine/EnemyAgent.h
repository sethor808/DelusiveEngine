#pragma once
#include "Agent.h"
#include "BehaviourScript.h"
#include "DelusiveRegistry.h"

class EnemyAgent : public Agent {
public:
	EnemyAgent(const std::string& agentName);

	//Overrides
	std::unique_ptr<Agent> Clone() const override;
	void Update(float deltaTime) override;
	void Draw(const glm::mat4& projection) const override;
	void DrawImGui() override;
	void OnHit() override;
	std::string GetType() const override;
	void RegisterProperties() override;

	//EnemyAgent logic
	void SetScript(const std::string&);
	void SetTarget(Agent*);

private:
	Agent* target = nullptr;
	std::string scriptName;
	

	glm::vec2 velocity = { 0.0f, 0.0f };
	glm::vec2 acceleration = { 0.0f, 0.0f };
	glm::vec2 impulse = { 0.0f, 0.0f };

	float moveSpeed = 10.0f;
	float damping = 0.9f;
};