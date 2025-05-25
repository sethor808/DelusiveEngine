#include "HurtboxCollider.h"

ShapeType HurtboxCollider::GetShapeType() const {
	return ShapeType::Box;
	//TODO: Make this variable
}

void HurtboxCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	if (col->GetColliderType() == ColliderType::Hitbox) {
		std::cout << "[Hurtbox] Damaging enemy agent." << std::endl;
		//Call damage here
	}
}