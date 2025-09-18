#pragma once
#include <memory>
#include <glm/glm.hpp>

class DelusiveScriptAgent;

class BehaviourScript {
public:
	BehaviourScript() = default;
	virtual ~BehaviourScript() = default;
	virtual void Update(float deltaTime) = 0;
	virtual std::unique_ptr<BehaviourScript> Clone() const = 0;

	//Getters and Setters
	virtual DelusiveScriptAgent* GetOwner() const { return owner; }
	virtual DelusiveScriptAgent* GetTarget() const { return target; }
	virtual void SetTarget(DelusiveScriptAgent* _target) { target = _target; }
	virtual void SetMovementSpeed(float speed) { movementSpeed = speed; }
	virtual float GetMovementSpeed() const { return movementSpeed; }
protected:
	DelusiveScriptAgent* owner = nullptr;
	DelusiveScriptAgent* target = nullptr;
	glm::vec2 direction = { 0,0 };
	float movementSpeed = 1.0f;
};