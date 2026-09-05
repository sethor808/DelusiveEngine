#pragma once
#include <Delusive/Runtime/Components/ColliderComponent.h>
#include <iostream>

class HitboxCollider : public ColliderComponent {
public:
	HitboxCollider(DelusiveInstance&);
	HitboxCollider() = delete;

	std::unique_ptr<Component> Clone() const override {
		//TODO: Properly deep copy values
		return std::make_unique<HitboxCollider>(instance);
	}

	ColliderType GetColliderType() const override {
		return ColliderType::Hitbox;
	}

	void Update(float) override {};
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "HitboxCollider";
	}
};