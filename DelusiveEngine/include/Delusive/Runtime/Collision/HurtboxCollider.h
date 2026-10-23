#pragma once
#include <Delusive/Runtime/Components/ColliderComponent.h>

class HurtboxCollider : public ColliderComponent {
public:
	HurtboxCollider(DelusiveInstance&);
	HurtboxCollider() = delete;

	ColliderType GetColliderType() const override {
		return ColliderType::Hurtbox;
	}

	const char* GetType() const override {
		return "HurtboxCollider";
	}

	void Update(float) override {};
	void OnCollision(ColliderComponent*) override;
};