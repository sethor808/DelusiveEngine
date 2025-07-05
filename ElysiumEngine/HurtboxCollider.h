#pragma once
#include "ColliderComponent.h"
#include <iostream>

class HurtboxCollider : public ColliderComponent {
public:
	std::unique_ptr<Component> Clone() const override {
		return std::make_unique<HurtboxCollider>(*this);
	}

	ColliderType GetColliderType() const override {
		return ColliderType::Hurtbox;
	}

	ShapeType GetShapeType() const override;

	const char* GetType() const override {
		return "HurtboxCollider";
	}

	void Update(float) override {};
	void DrawImGui() override;
	void OnCollision(ColliderComponent*) override;

	void Serialize(std::ofstream& out) const override;
	void Deserialize(std::ifstream& in) override;
};