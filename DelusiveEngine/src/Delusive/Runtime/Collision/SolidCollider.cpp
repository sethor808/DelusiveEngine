#include <Delusive/Runtime/Collision/SolidCollider.h>

SolidCollider::SolidCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{

}

void SolidCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	std::cout << "[Solid] collided with another object." << std::endl;
}