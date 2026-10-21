#pragma once
#include <Delusive/Runtime/Components/ColliderComponent.h>

class HitboxCollider : public ColliderComponent {
public:
	HitboxCollider(DelusiveInstance&);
	HitboxCollider() = delete;

	ColliderType GetColliderType() const override {
		return ColliderType::Hitbox;
	}

	void RegisterProperties() override;
	void Update(float) override {};
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "HitboxCollider";
	}

	int GetDamage() const { return damage; }
	bool IsParryable() const { return parryable; }

private:
	int damage = 1;
	bool parryable = true;
};
