#pragma once
#include <memory>
#include <glm/glm.hpp>

class Agent;

class BehaviourScript {
public:
	BehaviourScript() = default;
	virtual ~BehaviourScript() = default;
	virtual void Update(float deltaTime) = 0;
	virtual std::unique_ptr<BehaviourScript> Clone() const = 0;

	//Getters and Setters
	virtual Agent* GetOwner() const { return owner; }
    virtual void SetOwner(Agent* scriptAgent) { owner = scriptAgent; }
	virtual Agent* GetTarget() const { return target; }
	virtual void SetTarget(Agent* _target) { target = _target; }
	virtual void SetMovementSpeed(float speed) { movementSpeed = speed; }
	virtual float GetMovementSpeed() const { return movementSpeed; }
protected:
    Agent* owner = nullptr;
    Agent* target = nullptr;
	glm::vec2 direction = { 0,0 };
	float movementSpeed = 1.0f;
};