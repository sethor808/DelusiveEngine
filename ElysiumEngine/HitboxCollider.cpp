#include "HitboxCollider.h"

ShapeType HitboxCollider::GetShapeType() const{
	return ShapeType::Box;
	//TODO: Make this variable
}

void HitboxCollider::OnCollision(ColliderComponent* col){
	if (col->GetType() == ColliderType::Hurtbox) {
		std::cout << "[Hitbox] Damaged by enemy agent." << std::endl;
		//Call damage here
	}
}