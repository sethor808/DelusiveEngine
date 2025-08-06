#pragma once
#include "ColliderComponent.h"
#include <iostream>

class TriggerCollider : public ColliderComponent {
public:
	std::unique_ptr<Component> Clone() const override {
		return std::make_unique<TriggerCollider>(*this);
	}

	ColliderType GetColliderType() const override {
		return ColliderType::Trigger;
	}

	void Update(float) override {};
	void DrawImGui() override;
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "TriggerCollider";
	}

	virtual void Serialize(std::ofstream& out) const override;
	virtual void Deserialize(std::ifstream& in) override;
private:
};