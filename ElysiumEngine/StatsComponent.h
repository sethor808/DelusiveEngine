#pragma once
#include "Component.h"

class StatsComponent : public Component {
public:
	std::unique_ptr<Component> Clone() const override {
		return std::make_unique<StatsComponent>(*this);
	}

	int GetHealth();
	int TakeDamage(int damage);
	void Heal(int heal);

	void Update(float) override;
	const char* GetType() const override {
		return "StatsComponent";
	}


	void Serialize(std::ofstream& out) const override;
	void Deserialize(std::ifstream& in) override;
private:
	int maxHealth, currentHealth;
};
