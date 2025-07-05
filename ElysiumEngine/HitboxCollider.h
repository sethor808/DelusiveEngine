#pragma once
#include "ColliderComponent.h"
#include <iostream>

class HitboxCollider : public ColliderComponent {
public:
	std::unique_ptr<Component> Clone() const override {
		return std::make_unique<HitboxCollider>(*this);
	}

	ColliderType GetColliderType() const override {
		return ColliderType::Hitbox;
	}

	ShapeType GetShapeType() const override;

	void Update(float) override {};
	void DrawImGui() override;
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "HitboxCollider";
	}

	void Serialize(std::ofstream& out) const override;
	void Deserialize(std::ifstream& in) override;
};