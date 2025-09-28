#include "HitboxCollider.h"

HitboxCollider::HitboxCollider(DelusiveRenderer& renderer) 
	: ColliderComponent(renderer)
{

}

void HitboxCollider::OnCollision(ColliderComponent* col){
	if (col->GetColliderType() == ColliderType::Hurtbox) {
		std::cout << "[Hitbox] Damaged by enemy agent." << std::endl;
		//Call damage here
	}
}