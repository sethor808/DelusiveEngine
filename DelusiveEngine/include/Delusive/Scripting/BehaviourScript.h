#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <Delusive/Runtime/Utils/UUID.h>

class Agent;

class BehaviourScript {
public:
	BehaviourScript() = default;
	virtual ~BehaviourScript() = default;
	virtual void Update(float deltaTime) = 0;
	virtual std::unique_ptr<BehaviourScript> Clone() const = 0;
    void CopyCore(const BehaviourScript*);
    virtual void RelocateReferences();

	//Getters and Setters
	virtual Agent* GetOwner() const { return owner; }
    virtual void SetOwner(Agent* scriptAgent) { owner = scriptAgent; }
	virtual Agent* GetTarget() const { return target; }
    UUID GetTargetID() const { return targetID; }
    virtual void SetTarget(Agent* _target);
	virtual void SetMovementSpeed(float speed) { movementSpeed = speed; }
	virtual float GetMovementSpeed() const { return movementSpeed; }
protected:
    Agent* owner = nullptr;
    Agent* target = nullptr;
    UUID targetID;
    std::unique_ptr<PropertyRegistry> registry;
	glm::vec2 direction = { 0,0 };
	float movementSpeed = 1.0f;
};