#pragma once
#include "Component.h"

class StatsComponent {
public:
	int GetHealth();
	bool TakeDamage(int damage);
	void Heal(int heal);

private:
	int maxHealth, currentHealth;
};
