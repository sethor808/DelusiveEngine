#pragma once
#include <Delusive/Runtime/Components/Component.h>

class StatsComponent : public Component {
public:
	StatsComponent(DelusiveInstance&);
	StatsComponent() = delete;

	int GetHealth();
	int TakeDamage(int damage);
	void Heal(int heal);

	void Update(float) override;
	const char* GetType() const override {
		return "StatsComponent";
	}
private:
	int maxHealth, currentHealth;
};
