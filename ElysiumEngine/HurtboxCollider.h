#pragma once
#include "ColliderComponent.h"
#include <iostream>

class HurtboxCollider : public ColliderComponent {
public:
	ColliderType GetColliderType() const override {
		return ColliderType::Hurtbox;
	}

	ShapeType GetShapeType() const override;

	void OnCollision(ColliderComponent*) override;
};