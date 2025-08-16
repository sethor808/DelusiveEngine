#include "PhysicsSystem.h"
#include "DelusiveComponents.h"

void PhysicsSystem::HandleCollisions(const std::vector<std::unique_ptr<Agent>>& agents) {
	for (size_t i = 0; i < agents.size(); ++i) {
		auto* agentA = agents[i].get();
		auto collidersA = agentA->GetComponentsOfType<ColliderComponent>();

		for (auto* colA : collidersA) {
			for (size_t j = i + 1; j < agents.size(); ++j) {
				auto* agentB = agents[j].get();
				auto collidersB = agentB->GetComponentsOfType<ColliderComponent>();

				for (auto* colB : collidersB) {
					if (colA->GetOwner() == colB->GetOwner()) continue;
					//printf("A type: %d, B type: %d\n", colA->GetColliderType(), colB->GetColliderType());

					bool isHit =
						(colA->GetColliderType() == ColliderType::Hitbox && colB->GetColliderType() == ColliderType::Hurtbox)
						|| (colA->GetColliderType() == ColliderType::Hurtbox && colB->GetColliderType() == ColliderType::Hitbox)
						|| (colA->GetColliderType() == ColliderType::Solid && (colB->GetColliderType() == ColliderType::Solid || colB->GetColliderType() == ColliderType::Trigger))
						|| (colA->GetColliderType() == ColliderType::Trigger && colB->GetColliderType() == ColliderType::Solid)
						;

 					if (isHit && CheckAABBCollision(colA, colB)) {
						//Notify
						colA->OnCollision(colB);
						colB->OnCollision(colA);

						if (colA->GetColliderType() == ColliderType::Solid) {
							ResolveSolidCollision(colA, colB);
						}
						else if (colB->GetColliderType() == ColliderType::Solid) {
							ResolveSolidCollision(colB, colA);
						}
					}
				}
			}
		}
	}
}

void PhysicsSystem::ResolveSolidCollision(ColliderComponent* solid, ColliderComponent* other) {
	glm::vec2 sMin = solid->GetMin(), sMax = solid->GetMax();
	glm::vec2 mMin = other->GetMin(), mMax = other->GetMax();
	glm::vec2 overlapMin = glm::max(sMin, mMin);
	glm::vec2 overlapMax = glm::min(sMax, mMax);
	glm::vec2 overlap = overlapMax - overlapMin;
	if (overlap.x <= 0 || overlap.y <= 0) return;

	glm::vec2 delta = (overlap.x < overlap.y)
		? glm::vec2((mMin.x < sMin.x) ? -overlap.x : overlap.x, 0.0f)
		: glm::vec2(0.0f, (mMin.y < sMin.y) ? -overlap.y : overlap.y);

	solid->GetOwner()->transform.position -= delta;
}

bool PhysicsSystem::CheckAABBCollision(ColliderComponent* a, ColliderComponent* b) {
	ShapeType sa = a->GetShapeType();
	ShapeType sb = b->GetShapeType();

	if (sa == ShapeType::Box && sb == ShapeType::Box)
		return CheckBoxBoxCollision(a, b);
	if (sa == ShapeType::Circle && sb == ShapeType::Circle)
		return CheckCircleCircleCollision(a, b);
	if (sa == ShapeType::Box && sb == ShapeType::Circle)
		return CheckBoxCircleCollision(a, b);
	if (sa == ShapeType::Circle && sb == ShapeType::Box)
		return CheckBoxCircleCollision(b, a);

	if (sa == ShapeType::Line && sb == ShapeType::Line)
		return CheckLineLineCollision(a, b);
	if (sa == ShapeType::Line && sb == ShapeType::Circle)
		return CheckLineCircleCollision(a, b);
	if (sa == ShapeType::Circle && sb == ShapeType::Line)
		return CheckLineCircleCollision(b, a);
	if (sa == ShapeType::Line && sb == ShapeType::Box)
		return CheckLineBoxCollision(a, b);
	if (sa == ShapeType::Box && sb == ShapeType::Line)
		return CheckLineBoxCollision(b, a);

	return false;
}

bool PhysicsSystem::CheckBoxBoxCollision(ColliderComponent* a, ColliderComponent* b) {
	glm::vec2 aMin = a->GetMin();
	glm::vec2 aMax = a->GetMax();
	glm::vec2 bMin = b->GetMin();
	glm::vec2 bMax = b->GetMax();

	return (aMin.x < bMax.x && aMax.x > bMin.x &&
		aMin.y < bMax.y && aMax.y > bMin.y);
}

bool PhysicsSystem::CheckCircleCircleCollision(ColliderComponent* circle, ColliderComponent* box) {
	glm::vec2 centerA = circle->transform.position;
	glm::vec2 centerB = box->transform.position;
	float radiusA = circle->transform.scale.x * 0.5f;
	float radiusB = box->transform.scale.x * 0.5f;

	float distSq = glm::length2(centerA - centerB);
	float radiusSum = radiusA + radiusB;

	return distSq <= radiusSum * radiusSum;
}

bool PhysicsSystem::CheckBoxCircleCollision(ColliderComponent* box, ColliderComponent* circle) {
	glm::vec2 boxMin = box->GetMin();
	glm::vec2 boxMax = box->GetMax();
	glm::vec2 circleCenter = circle->transform.position;
	float radius = circle->transform.scale.x * 0.5f;

	// Clamp circle center to nearest point inside box
	glm::vec2 closest = glm::clamp(circleCenter, boxMin, boxMax);
	glm::vec2 delta = circleCenter - closest;

	return glm::length2(delta) <= radius * radius;
}

bool PhysicsSystem::CheckLineLineCollision(ColliderComponent* a, ColliderComponent* b) {
	glm::vec2 p1 = a->transform.position;
	glm::vec2 d1 = glm::vec2(cos(a->transform.rotation), sin(a->transform.rotation)) * a->transform.scale.x;
	glm::vec2 p2 = b->transform.position;
	glm::vec2 d2 = glm::vec2(cos(b->transform.rotation), sin(b->transform.rotation)) * b->transform.scale.x;

	glm::vec2 q1 = p1 + d1;
	glm::vec2 q2 = p2 + d2;

	// Line segment intersection check
	auto ccw = [](glm::vec2 A, glm::vec2 B, glm::vec2 C) {
		return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
		};

	return (ccw(p1, p2, q2) != ccw(q1, p2, q2)) &&
		(ccw(p1, q1, p2) != ccw(p1, q1, q2));
}

bool PhysicsSystem::CheckLineCircleCollision(ColliderComponent* line, ColliderComponent* circle) {
	glm::vec2 p1 = line->transform.position;
	glm::vec2 dir = glm::vec2(cos(line->transform.rotation), sin(line->transform.rotation));
	glm::vec2 p2 = p1 + dir * line->transform.scale.x;

	glm::vec2 circleCenter = circle->transform.position;
	float radius = circle->transform.scale.x * 0.5f;

	// Project point onto segment
	glm::vec2 seg = p2 - p1;
	glm::vec2 toCenter = circleCenter - p1;

	float t = glm::clamp(glm::dot(toCenter, seg) / glm::dot(seg, seg), 0.0f, 1.0f);
	glm::vec2 closest = p1 + seg * t;

	glm::vec2 delta = circleCenter - closest;
	return glm::length2(delta) <= radius * radius;
}

bool PhysicsSystem::CheckLineBoxCollision(ColliderComponent* line, ColliderComponent* box) {
	glm::vec2 p1 = line->transform.position;
	glm::vec2 dir = glm::vec2(cos(line->transform.rotation), sin(line->transform.rotation));
	glm::vec2 p2 = p1 + dir * line->transform.scale.x;

	glm::vec2 boxMin = box->GetMin();
	glm::vec2 boxMax = box->GetMax();

	// Check for intersection with each of the 4 box edges
	auto IntersectsSegment = [](glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d) {
		auto ccw = [](glm::vec2 A, glm::vec2 B, glm::vec2 C) {
			return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
			};
		return (ccw(a, c, d) != ccw(b, c, d)) && (ccw(a, b, c) != ccw(a, b, d));
		};

	glm::vec2 bl = { boxMin.x, boxMin.y };
	glm::vec2 br = { boxMax.x, boxMin.y };
	glm::vec2 tr = { boxMax.x, boxMax.y };
	glm::vec2 tl = { boxMin.x, boxMax.y };

	return IntersectsSegment(p1, p2, bl, br) ||
		IntersectsSegment(p1, p2, br, tr) ||
		IntersectsSegment(p1, p2, tr, tl) ||
		IntersectsSegment(p1, p2, tl, bl);
}