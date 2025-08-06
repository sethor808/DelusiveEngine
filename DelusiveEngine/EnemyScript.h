#pragma once
class EnemyAgent;

class EnemyScript {
public:
	virtual ~EnemyScript() = default;
	virtual void Update(EnemyAgent* owner, float deltaTime) = 0;
	virtual std::unique_ptr<EnemyScript> Clone() const = 0;
};