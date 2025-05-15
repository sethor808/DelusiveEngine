#pragma once
#include "ColliderComponent.h"
#include <iostream>

class HitboxCollider : public ColliderComponent {
public:
	ColliderType GetType() const override {
		return ColliderType::Hitbox;
	}

	ShapeType GetShapeType() const override;

	void OnCollision(ColliderComponent*) override;
};