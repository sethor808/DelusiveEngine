#include <Delusive/Runtime/Collision/HitboxCollider.h>

HitboxCollider::HitboxCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{
    RegisterProperties();
}

void HitboxCollider::OnCollision(ColliderComponent*){
	//The hurtbox side applies the damage
}
