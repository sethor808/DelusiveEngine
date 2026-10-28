#pragma once
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Scripting/BehaviourScript.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>

class EnemyAgent : public Agent {
public:
	EnemyAgent(DelusiveInstance&);
	EnemyAgent() = delete;

	//Overrides
	void Update(float deltaTime) override;
	void Draw(const glm::mat4& projection) const override;
	void DrawImGui() ;
	std::string GetType() const override;
	void RegisterProperties() override;

	//EnemyAgent logic
	void SetTarget(Agent*);

	//Combat
	void HitEvent(const HitData&) override;
	int GetHealth() const { return maxHealth - damageTaken; }

private:
	Agent* target = nullptr;

	int maxHealth = 3;
	int damageTaken = 0;

	glm::vec2 velocity = { 0.0f, 0.0f };
	glm::vec2 acceleration = { 0.0f, 0.0f };
	glm::vec2 impulse = { 0.0f, 0.0f };

	float moveSpeed = 10.0f;
	float damping = 0.9f;
};