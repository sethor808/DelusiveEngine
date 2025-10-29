#pragma once
#include <DelusiveInternal/Components/ColliderComponent.h>
#include <iostream>

class HurtboxCollider : public ColliderComponent {
public:
	HurtboxCollider(DelusiveRenderer&);
	HurtboxCollider() = delete;

	std::unique_ptr<Component> Clone() const override {
		//TODO: Properly copy over values
		return std::make_unique<HurtboxCollider>(renderer);
	}

	ColliderType GetColliderType() const override {
		return ColliderType::Hurtbox;
	}

	const char* GetType() const override {
		return "HurtboxCollider";
	}

	void Update(float) override {};
	void OnCollision(ColliderComponent*) override;

	//void Serialize(std::ofstream& out) const override;
	//void Deserialize(std::ifstream& in) override;
};