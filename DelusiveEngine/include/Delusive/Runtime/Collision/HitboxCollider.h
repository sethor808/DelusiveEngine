#pragma once
#include <Delusive/Runtime/Components/ColliderComponent.h>
#include <iostream>

class HitboxCollider : public ColliderComponent {
public:
	HitboxCollider(DelusiveInstance&);
	HitboxCollider() = delete;

	ColliderType GetColliderType() const override {
		return ColliderType::Hitbox;
	}

	void Update(float) override {};
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "HitboxCollider";
	}
};