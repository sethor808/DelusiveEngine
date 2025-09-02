#pragma once
class EnemyAgent;

class BehaviourScript {
public:
	virtual ~BehaviourScript() = default;
	virtual void Update(EnemyAgent* owner, float deltaTime) = 0;
	virtual std::unique_ptr<BehaviourScript> Clone() const = 0;
};