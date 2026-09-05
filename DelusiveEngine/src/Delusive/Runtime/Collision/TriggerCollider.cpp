#include <Delusive/Runtime/Collision/TriggerCollider.h>

TriggerCollider::TriggerCollider(DelusiveInstance& instance)
    : ColliderComponent(instance)
{

}

void TriggerCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	if (col->GetColliderType() == ColliderType::Solid) {
		std::cout << "[Trigger] occurred by solid collider." << std::endl;
		//Call damage here
	}
}