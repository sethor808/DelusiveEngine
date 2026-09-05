#pragma once
//#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <memory>
#include <glm/glm.hpp>
#include <Delusive/Runtime/Core/IDLink.h>

class Agent;
class PropertyRegistry;

class BehaviourScript {
public:
    BehaviourScript();
	virtual ~BehaviourScript();

    BehaviourScript(const BehaviourScript&) = delete;
    BehaviourScript& operator=(const BehaviourScript&) = delete;
    BehaviourScript(BehaviourScript&&) noexcept = default;
    BehaviourScript& operator=(BehaviourScript&&) noexcept = default;

    void DrawImGui();
	virtual void Update(float deltaTime);
	virtual std::unique_ptr<BehaviourScript> Clone() const = 0;
    virtual std::string GetType() = 0;
    void CopyCore(const BehaviourScript*);
    virtual void RelocateReferences();

	//Getters and Setters
	virtual Agent* GetOwner() const { return owner.get(); }
    virtual void SetOwner(Agent*);
	virtual Agent* GetTarget() const { return target.get(); }
    UUID GetTargetID() const;
    virtual void SetTarget(Agent* _target);
	virtual void SetMovementSpeed(float speed) { movementSpeed = speed; }
	virtual float GetMovementSpeed() const { return movementSpeed; }

    //Serialization
    void Serialize(std::ostream&) const;
    void Deserialize(DelusiveParser::DataBlock&);

    virtual void RegisterProperties();
protected:
    DelusiveLink<Agent> owner;
    DelusiveLink<Agent> target;
    DelusiveLink<Agent> targetID;

    //UUID targetID;
    std::unique_ptr<PropertyRegistry> registry;
	glm::vec2 direction = { 0,0 };
	float movementSpeed = 1.0f;
};