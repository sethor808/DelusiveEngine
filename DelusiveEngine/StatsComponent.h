#pragma once
#include "Component.h"

class StatsComponent : public Component {
public:
	StatsComponent(DelusiveRenderer&);
	StatsComponent() = delete;

	std::unique_ptr<Component> Clone() const override {
		//TODO: Properly copy over values
		return std::make_unique<StatsComponent>(renderer);
	}

	int GetHealth();
	int TakeDamage(int damage);
	void Heal(int heal);

	void Update(float) override;
	const char* GetType() const override {
		return "StatsComponent";
	}

	//void Serialize(std::ofstream& out) const override;
	//void Deserialize(std::ifstream& in) override;
private:
	int maxHealth, currentHealth;
};
