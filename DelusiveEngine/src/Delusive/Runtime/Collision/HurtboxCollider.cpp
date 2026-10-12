#include <Delusive/Runtime/Collision/HurtboxCollider.h>
#include <Delusive/Runtime/Collision/HitboxCollider.h>
#include <Delusive/Runtime/Collision/HitData.h>

HurtboxCollider::HurtboxCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{
    RegisterProperties();
}

void HurtboxCollider::OnCollision(ColliderComponent* col) {
	if (col->GetColliderType() != ColliderType::Hitbox) return;

	const auto* hitbox = static_cast<const HitboxCollider*>(col);
	Agent* attacker = hitbox->GetOwner();
	Agent* target = GetOwner();
	if (!attacker || !target) return;

	HitData hit;
	hit.attacker = attacker;
	hit.damage = hitbox->GetDamage();
	hit.parryable = hitbox->IsParryable();

	glm::vec2 offset = target->GetTransform().position - attacker->GetTransform().position;
	if (glm::length(offset) > 0.0f) hit.direction = glm::normalize(offset);

	target->HitEvent(hit);
	attacker->LandHitEvent(hit);
}
