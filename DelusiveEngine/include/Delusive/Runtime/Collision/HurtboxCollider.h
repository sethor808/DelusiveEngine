#pragma once
#include <Delusive/Runtime/Components/ColliderComponent.h>
#include <iostream>

class HurtboxCollider : public ColliderComponent {
public:
	HurtboxCollider(DelusiveInstance&);
	HurtboxCollider() = delete;

	std::unique_ptr<Component> Clone() const override {
		//TODO: Properly copy over values
		return std::make_unique<HurtboxCollider>(instance);
	}

	ColliderType GetColliderType() const override {
		return ColliderType::Hurtbox;
	}

	const char* GetType() const override {
		return "HurtboxCollider";
	}

	void Update(float) override {};
	void OnCollision(ColliderComponent*) override;
};