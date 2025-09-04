#include "HitboxCollider.h"

void HitboxCollider::OnCollision(ColliderComponent* col){
	if (col->GetColliderType() == ColliderType::Hurtbox) {
		std::cout << "[Hitbox] Damaged by enemy agent." << std::endl;
		//Call damage here
	}
}