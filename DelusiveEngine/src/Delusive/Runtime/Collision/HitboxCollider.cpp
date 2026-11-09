#include <Delusive/Runtime/Collision/HitboxCollider.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <algorithm>

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

void HitboxCollider::NewStrike() {
	++strike;
	struck.clear();
}

bool HitboxCollider::Strike(Agent* target) {
	//Placed hitboxes never start strikes and hit on every new contact
	if (strike == 0) return true;
	if (std::find(struck.begin(), struck.end(), target) != struck.end()) return false;
	struck.push_back(target);
	return true;
}
