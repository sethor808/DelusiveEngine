#pragma once
#include "ColliderComponent.h"
#include <iostream>

class TriggerCollider : public ColliderComponent {
public:
	std::unique_ptr<Component> Clone() const override {
		//TODO: Properly copy over values
		return std::make_unique<TriggerCollider>();
	}

	ColliderType GetColliderType() const override {
		return ColliderType::Trigger;
	}

	void Update(float) override {};
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "TriggerCollider";
	}

	//void Serialize(std::ofstream& out) const override;
	//void Deserialize(std::ifstream& in) override;
private:
};