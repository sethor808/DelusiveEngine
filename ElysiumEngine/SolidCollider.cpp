#include "SolidCollider.h"

ShapeType SolidCollider::GetShapeType() const {
	return ShapeType::Box;
	//TODO: Make this variable
}

void SolidCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	std::cout << "[Solid] collided with another object." << std::endl;
}