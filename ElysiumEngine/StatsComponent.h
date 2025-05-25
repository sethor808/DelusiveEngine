#pragma once
#include "Component.h"

class StatsComponent : public Component {
public:
	int GetHealth();
	int TakeDamage(int damage);
	void Heal(int heal);

	void Update(float) override;
	const char* GetType() const override {
		return "Stats";
	}

private:
	int maxHealth, currentHealth;
};
