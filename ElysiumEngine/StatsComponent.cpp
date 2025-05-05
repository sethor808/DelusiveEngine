#include "StatsComponent.h"

int StatsComponent::GetHealth() {
	return 0;
}

bool StatsComponent::TakeDamage(int damage) {
	currentHealth -= damage;
	
	if (currentHealth > maxHealth) {
		currentHealth = maxHealth;
	}

	if (currentHealth <= 0) {
		return -1;
	}
	else {
		return 0;
	}
}

void StatsComponent::Heal(int heal) {
	currentHealth += heal;

	if (currentHealth > maxHealth) {
		currentHealth = maxHealth;
	}
}