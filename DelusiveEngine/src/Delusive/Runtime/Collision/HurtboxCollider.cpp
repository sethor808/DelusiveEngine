#include <Delusive/Runtime/Collision/HurtboxCollider.h>

HurtboxCollider::HurtboxCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{
    RegisterProperties();
}

void HurtboxCollider::OnCollision(ColliderComponent* col) {
	if (col->GetColliderType() == ColliderType::Hitbox) {
		GetOwner()->TakeDamage();
	}
}
