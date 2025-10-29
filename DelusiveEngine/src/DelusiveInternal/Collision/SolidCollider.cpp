#include <DelusiveInternal/Collision/SolidCollider.h>

SolidCollider::SolidCollider(DelusiveRenderer& renderer)
	: ColliderComponent(renderer)
{

}

void SolidCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	std::cout << "[Solid] collided with another object." << std::endl;
}