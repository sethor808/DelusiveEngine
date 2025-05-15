#pragma once
#include "ColliderComponent.h"
#include <iostream>

class SolidCollider : public ColliderComponent {
public:
	ColliderType GetType() const override {
		return ColliderType::Solid;
	}

	ShapeType GetShapeType() const override;

	void OnCollision(ColliderComponent*) override;
};