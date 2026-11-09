#pragma once
#include <Delusive/Runtime/Components/ColliderComponent.h>
#include <vector>

class Agent;

class HitboxCollider : public ColliderComponent {
public:
	HitboxCollider(DelusiveInstance&);
	HitboxCollider() = delete;

	ColliderType GetColliderType() const override {
		return ColliderType::Hitbox;
	}

	void RegisterProperties() override;
	void Update(float) override {};
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "HitboxCollider";
	}

	int GetDamage() const { return damage; }
	bool IsParryable() const { return parryable; }

	//A strike is one attack (animator hitboxes start one per branch): it lands on each agent at most once
	int GetStrike() const override { return strike; }
	void NewStrike();
	//False if this strike already hit the agent
	bool Strike(Agent*);

private:
	int damage = 1;
	bool parryable = true;
	int strike = 0;
	std::vector<Agent*> struck;
};
