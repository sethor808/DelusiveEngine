#include <Delusive/Runtime/Collision/HurtboxCollider.h>
#include <Delusive/Runtime/Collision/HitboxCollider.h>
#include <Delusive/Runtime/Collision/HitData.h>
#include <Delusive/Runtime/Core/PhysicsSystem.h>
#include <Delusive/Runtime/Scene/Scene.h>

HurtboxCollider::HurtboxCollider(DelusiveInstance& instance)
	: ColliderComponent(instance)
{
    RegisterProperties();
}

void HurtboxCollider::OnCollision(ColliderComponent* col) {
	if (col->GetColliderType() != ColliderType::Hitbox) return;

	auto* hitbox = static_cast<HitboxCollider*>(col);
	Agent* attacker = hitbox->GetOwner();
	Agent* target = GetOwner();
	if (!attacker || !target) return;
	//An agent with several hurtboxes still takes one hit per strike
	if (!hitbox->Strike(target)) return;

	HitData hit;
	hit.attacker = attacker;
	hit.target = target;
	hit.damage = hitbox->GetDamage();
	hit.parryable = hitbox->IsParryable();

	glm::vec2 offset = target->GetTransform().position - attacker->GetTransform().position;
	if (glm::length(offset) > 0.0f) hit.direction = glm::normalize(offset);

	//Centre of where the two boxes overlap
	const WorldShape hitShape = PhysicsSystem::BuildShape(*hitbox);
	const WorldShape hurtShape = PhysicsSystem::BuildShape(*this);
	hit.point = (glm::max(hitShape.min, hurtShape.min) + glm::min(hitShape.max, hurtShape.max)) * 0.5f;

	attacker->ModifyOutgoingHit(hit);
	if (!target->HitEvent(hit)) return;
	attacker->LandHitEvent(hit);
	if (Scene* scene = target->GetScene()) scene->PlayHitEffects(hit);
}
