#include "SolidCollider.h"

void SolidCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	std::cout << "[Solid] collided with another object." << std::endl;
}