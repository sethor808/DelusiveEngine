#include "StatsComponent.h"
#include <fstream>

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

void StatsComponent::Serialize(std::ofstream& out) const {
	out << "Stats\n";
	out << currentHealth << "\n";
	out << maxHealth << "\n";
}

void StatsComponent::Deserialize(std::ifstream& in) {
	in >> currentHealth;
	in >> maxHealth;
	in.ignore();
}