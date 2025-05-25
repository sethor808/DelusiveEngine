#pragma once
#include "ColliderComponent.h"
#include <iostream>

class TriggerCollider : public ColliderComponent {
public:
	ColliderType GetColliderType() const override {
		return ColliderType::Trigger;
	}

	ShapeType GetShapeType() const override;

	void OnCollision(ColliderComponent*) override;
};