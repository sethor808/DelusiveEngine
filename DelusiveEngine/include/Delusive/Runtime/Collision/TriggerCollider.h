#pragma once
#include <Delusive/Runtime/Components/ColliderComponent.h>
#include <iostream>

class TriggerCollider : public ColliderComponent {
public:
	TriggerCollider(DelusiveInstance&);
	TriggerCollider() = delete;

	std::unique_ptr<Component> Clone() const override {
		//TODO: Properly copy over values
		return std::make_unique<TriggerCollider>(instance);
	}

	ColliderType GetColliderType() const override {
		return ColliderType::Trigger;
	}

	void Update(float) override {};
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "TriggerCollider";
	}
private:
};