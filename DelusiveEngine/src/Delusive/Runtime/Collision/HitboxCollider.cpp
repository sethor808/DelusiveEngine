#include <Delusive/Runtime/Collision/HitboxCollider.h>

HitboxCollider::HitboxCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{
    RegisterProperties();
}

void HitboxCollider::OnCollision(ColliderComponent* col){
	if (col->GetColliderType() == ColliderType::Hurtbox) {
		std::cout << "[Hitbox] Damaged by enemy agent." << std::endl;
		//Call damage here
	}
}