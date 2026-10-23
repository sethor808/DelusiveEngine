#include <Delusive/Runtime/Collision/SolidCollider.h>

SolidCollider::SolidCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{
    RegisterProperties();
}

void SolidCollider::OnCollision(ColliderComponent*) {
	//Separation is handled by PhysicsSystem
}