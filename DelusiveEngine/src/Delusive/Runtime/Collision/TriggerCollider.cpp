#include <Delusive/Runtime/Collision/TriggerCollider.h>
#include <iostream>

TriggerCollider::TriggerCollider(DelusiveInstance& instance)
    : ColliderComponent(instance)
{
    RegisterProperties();
}

void TriggerCollider::OnCollision(ColliderComponent* col) {
	if (col->GetColliderType() == ColliderType::Solid) {
		std::cout << "[Trigger] occurred by solid collider." << std::endl;
	}
}