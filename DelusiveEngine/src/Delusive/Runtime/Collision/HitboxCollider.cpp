#include <Delusive/Runtime/Collision/HitboxCollider.h>

HitboxCollider::HitboxCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{
    RegisterProperties();
}

void HitboxCollider::OnCollision(ColliderComponent*) {
	//Damage is applied by the hurtbox
}
