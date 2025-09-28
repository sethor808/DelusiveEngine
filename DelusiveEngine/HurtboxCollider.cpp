#include "HurtboxCollider.h"

HurtboxCollider::HurtboxCollider(DelusiveRenderer& renderer)
	: ColliderComponent(renderer)
{

}

void HurtboxCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	if (col->GetColliderType() == ColliderType::Hitbox) {
		std::cout << "[Hurtbox] Damaging enemy agent." << std::endl;
		//Call damage here
	}
}
