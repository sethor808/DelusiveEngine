#include <Delusive/Runtime/Collision/HitboxCollider.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>

HitboxCollider::HitboxCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{
    RegisterProperties();
}

void HitboxCollider::RegisterProperties() {
	ColliderComponent::RegisterProperties();
	registry->Register("damage", &damage);
	registry->Register("parryable", &parryable);
}

void HitboxCollider::OnCollision(ColliderComponent*) {
	//Damage is applied by the hurtbox
}
