#include "TriggerCollider.h"

void TriggerCollider::OnCollision(ColliderComponent* col) {
	//TODO: Prevent self hits
	if (col->GetColliderType() == ColliderType::Solid) {
		std::cout << "[Trigger] occurred by solid collider." << std::endl;
		//Call damage here
	}
}

/*
void TriggerCollider::Serialize(std::ofstream& out) const {
    out << "Trigger Collider\n";
    out << name << "\n";
    out << transform.position.x << " " << transform.position.y << "\n";
    out << transform.rotation << "\n";
    out << transform.scale.x << " " << transform.scale.y << "\n";
}

void TriggerCollider::Deserialize(std::ifstream& in) {
    in >> name;
    in >> transform.position.x >> transform.position.y;
    in >> transform.rotation;
    in >> transform.scale.x >> transform.scale.y;
    in.ignore();
}
*/