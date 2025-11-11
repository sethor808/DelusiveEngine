#pragma once
#include <DelusiveInternal/Agents/Agent.h>
#include <DelusiveExternal/BehaviourScript.h>
#include <DelusiveInternal/Core/DelusiveRegistry.h>

class EnemyAgent : public Agent {
public:
	EnemyAgent(DelusiveRenderer&);
	EnemyAgent() = delete;

	//Overrides
	std::unique_ptr<Agent> Clone(Scene*) const override;
	void Update(float deltaTime);
	void Draw(const glm::mat4& projection) const override;
	void DrawImGui() ;
	void OnHit() override;
	std::string GetType() const override;
	void RegisterProperties() override;

	//EnemyAgent logic
	void SetTarget(Agent*);

private:
	Agent* target = nullptr;
	

	glm::vec2 velocity = { 0.0f, 0.0f };
	glm::vec2 acceleration = { 0.0f, 0.0f };
	glm::vec2 impulse = { 0.0f, 0.0f };

	float moveSpeed = 10.0f;
	float damping = 0.9f;
};