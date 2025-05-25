#include "StatsComponent.h"

int StatsComponent::GetHealth() {
	return 0;
}

int StatsComponent::TakeDamage(int damage) {
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

void StatsComponent::Update(float deltaTime) {

}