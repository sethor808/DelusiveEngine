#pragma once
#include <Delusive/Runtime/Components/ColliderComponent.h>

class TriggerCollider : public ColliderComponent {
public:
	TriggerCollider(DelusiveInstance&);
	TriggerCollider() = delete;

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