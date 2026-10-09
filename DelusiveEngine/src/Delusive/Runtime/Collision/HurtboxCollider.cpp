#include <Delusive/Runtime/Collision/HurtboxCollider.h>

HurtboxCollider::HurtboxCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{
    RegisterProperties();
}

void HurtboxCollider::OnCollision(ColliderComponent* col) {
	//Self and same team hits are filtered out by PhysicsSystem
	if (col->GetColliderType() == ColliderType::Hitbox) {
		GetOwner()->TakeDamage();
	}
}
