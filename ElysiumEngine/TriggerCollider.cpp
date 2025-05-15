#include "TriggerCollider.h"

ShapeType TriggerCollider::GetShapeType() const {
	return ShapeType::Box;
	//TODO: Make this variable
}

void TriggerCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	if (col->GetType() == ColliderType::Solid) {
		std::cout << "[Trigger] occured by solid collider." << std::endl;
		//Call damage here
	}
}