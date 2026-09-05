#include <Delusive/Runtime/Collision/HurtboxCollider.h>

HurtboxCollider::HurtboxCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{

}

void HurtboxCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	if (col->GetColliderType() == ColliderType::Hitbox) {
		std::cout << "[Hurtbox] Damaging enemy agent." << std::endl;
		//Call damage here
	}
}
