#pragma once
#include "ColliderComponent.h"
#include <iostream>

class SolidCollider : public ColliderComponent {
public:
	ColliderType GetColliderType() const override {
		return ColliderType::Solid;
	}

	ShapeType GetShapeType() const override;

	void Update(float) override{};
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "Solid Collider";
	}
};