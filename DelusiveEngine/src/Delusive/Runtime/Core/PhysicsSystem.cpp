#include <Delusive/Runtime/Core/PhysicsSystem.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace {
	float LengthSq(const glm::vec2& v) { return glm::dot(v, v); }

	bool Contains(const WorldShape& box, const glm::vec2& p) {
		return p.x > box.min.x && p.x < box.max.x && p.y > box.min.y && p.y < box.max.y;
	}

	//Proper crossing of segments ab and cd
	bool SegmentsCross(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d) {
		auto ccw = [](glm::vec2 A, glm::vec2 B, glm::vec2 C) {
			return (C.y - A.y) * (B.x - A.x) > (B.y - A.y) * (C.x - A.x);
		};
		return (ccw(a, c, d) != ccw(b, c, d)) && (ccw(a, b, c) != ccw(a, b, d));
	}

	glm::vec2 ClosestOnSegment(glm::vec2 a, glm::vec2 b, glm::vec2 p) {
		const glm::vec2 seg = b - a;
		const float lengthSq = LengthSq(seg);
		if (lengthSq == 0.0f) return a; //A zero length line is a point
		const float t = std::clamp(glm::dot(p - a, seg) / lengthSq, 0.0f, 1.0f);
		return a + seg * t;
	}

	bool BoxBox(const WorldShape& a, const WorldShape& b) {
		return a.min.x < b.max.x && a.max.x > b.min.x &&
			a.min.y < b.max.y && a.max.y > b.min.y;
	}

	bool CircleCircle(const WorldShape& a, const WorldShape& b) {
		const float reach = a.radius + b.radius;
		return LengthSq(a.center - b.center) < reach * reach;
	}

	bool BoxCircle(const WorldShape& box, const WorldShape& circle) {
		const glm::vec2 closest = glm::clamp(circle.center, box.min, box.max);
		return LengthSq(circle.center - closest) < circle.radius * circle.radius;
	}

	bool LineLine(const WorldShape& a, const WorldShape& b) {
		return SegmentsCross(a.center, a.end, b.center, b.end);
	}

	bool LineCircle(const WorldShape& line, const WorldShape& circle) {
		const glm::vec2 closest = ClosestOnSegment(line.center, line.end, circle.center);
		return LengthSq(circle.center - closest) < circle.radius * circle.radius;
	}

	bool LineBox(const WorldShape& line, const WorldShape& box) {
		//A segment entirely inside the box crosses no edge
		if (Contains(box, line.center) || Contains(box, line.end)) return true;

		const glm::vec2 bl = box.min, tr = box.max;
		const glm::vec2 br = { tr.x, bl.y }, tl = { bl.x, tr.y };
		return SegmentsCross(line.center, line.end, bl, br) ||
			SegmentsCross(line.center, line.end, br, tr) ||
			SegmentsCross(line.center, line.end, tr, tl) ||
			SegmentsCross(line.center, line.end, tl, bl);
	}
}

WorldShape PhysicsSystem::BuildShape(const ColliderComponent& collider) {
	WorldShape out;
	out.collider = const_cast<ColliderComponent*>(&collider);
	out.owner = collider.GetOwner();
	out.type = collider.GetColliderType();
	out.shape = collider.GetShapeType();

	//Colliders never rotate: world = agent position + agent scale * local. A negative
	//agent scale (facing left) mirrors the offset, so hitboxes flip with the agent.
	const Transform& agent = out.owner->GetTransform();
	const Transform& local = *collider.transform;
	const glm::vec2 center = agent.position + agent.scale * local.position;
	const glm::vec2 size = glm::abs(agent.scale * local.scale);

	switch (out.shape) {
	case ShapeType::Circle: {
		out.center = center;
		out.radius = 0.5f * std::abs(local.scale.x) * std::max(std::abs(agent.scale.x), std::abs(agent.scale.y));
		out.min = center - glm::vec2(out.radius);
		out.max = center + glm::vec2(out.radius);
		break;
	}
	case ShapeType::Line: {
		//A line is the one shape with a direction: local rotation, length scale.x
		const glm::vec2 dir = { std::cos(local.rotation), std::sin(local.rotation) };
		out.center = center;
		out.end = center + agent.scale * (dir * local.scale.x);
		out.min = glm::min(out.center, out.end);
		out.max = glm::max(out.center, out.end);
		break;
	}
	case ShapeType::Box:
	default: {
		out.center = center;
		out.min = center - 0.5f * size;
		out.max = center + 0.5f * size;
		break;
	}
	}

	return out;
}

bool PhysicsSystem::Interacts(ColliderType a, ColliderType b) {
	//Which collider types meet - one table instead of a chain of conditions
	static constexpr bool table[4][4] = {
		//             Solid  Hitbox Hurtbox Trigger
		/*Solid*/    { true,  false, false,  true  },
		/*Hitbox*/   { false, false, true,   false },
		/*Hurtbox*/  { false, true,  false,  false },
		/*Trigger*/  { true,  false, false,  false },
	};
	return table[static_cast<int>(a)][static_cast<int>(b)];
}

bool PhysicsSystem::Overlaps(const WorldShape& a, const WorldShape& b) {
	//Order the pair so each shape combination has one test
	if (static_cast<int>(a.shape) > static_cast<int>(b.shape)) return Overlaps(b, a);

	switch (a.shape) {
	case ShapeType::Box:
		if (b.shape == ShapeType::Box)    return BoxBox(a, b);
		if (b.shape == ShapeType::Circle) return BoxCircle(a, b);
		return LineBox(b, a);
	case ShapeType::Circle:
		if (b.shape == ShapeType::Circle) return CircleCircle(a, b);
		return LineCircle(b, a);
	case ShapeType::Line:
		return LineLine(a, b);
	}
	return false;
}

void PhysicsSystem::Step(const std::vector<std::unique_ptr<Agent>>& agents) {
	//1. Gather - every enabled collider resolved to world space once
	shapes.clear();
	for (const auto& agent : agents) {
		for (ColliderComponent* collider : agent->GetComponentsOfType<ColliderComponent>()) {
			if (collider->IsEnabled()) shapes.push_back(BuildShape(*collider));
		}
	}

	//2. Find every contact against this tick's positions before anything reacts
	contacts.clear();
	for (size_t i = 0; i < shapes.size(); ++i) {
		for (size_t j = i + 1; j < shapes.size(); ++j) {
			const WorldShape& a = shapes[i];
			const WorldShape& b = shapes[j];

			if (a.owner == b.owner) continue; //An agent never collides with itself
			if (!Interacts(a.type, b.type)) continue;
			if (a.max.x < b.min.x || b.max.x < a.min.x ||
				a.max.y < b.min.y || b.max.y < a.min.y) continue; //Bounds reject
			if (!Overlaps(a, b)) continue;

			contacts.push_back({ i, j });
		}
	}

	//3. React - callbacks first, then separation, all from the same snapshot
	for (const Contact& contact : contacts) {
		shapes[contact.a].collider->OnCollision(shapes[contact.b].collider);
		shapes[contact.b].collider->OnCollision(shapes[contact.a].collider);
	}

	ResolveSolids();
}

void PhysicsSystem::ResolveSolids() {
	//Static solids (walls) stop everything else. Two moving solids pass through each
	//other - characters meet through hit/hurt contact rather than shoving.
	//Largest push per axis per agent, so standing against two wall pieces at once does
	//not double the correction.
	std::unordered_map<Agent*, glm::vec2> pushes;

	for (const Contact& contact : contacts) {
		const WorldShape& a = shapes[contact.a];
		const WorldShape& b = shapes[contact.b];
		if (a.type != ColliderType::Solid || b.type != ColliderType::Solid) continue;

		const bool aStatic = a.owner->IsStatic();
		const bool bStatic = b.owner->IsStatic();
		if (aStatic == bStatic) continue;

		const WorldShape& mover = aStatic ? b : a;
		const WorldShape& wall = aStatic ? a : b;

		//Shortest way out of the wall, along the shallower axis
		const glm::vec2 overlap = glm::min(mover.max, wall.max) - glm::max(mover.min, wall.min);
		if (overlap.x <= 0.0f || overlap.y <= 0.0f) continue;

		const glm::vec2 moverCenter = mover.min + mover.max; //Doubled - only compared
		const glm::vec2 wallCenter = wall.min + wall.max;
		const glm::vec2 push = (overlap.x < overlap.y)
			? glm::vec2(moverCenter.x < wallCenter.x ? -overlap.x : overlap.x, 0.0f)
			: glm::vec2(0.0f, moverCenter.y < wallCenter.y ? -overlap.y : overlap.y);

		glm::vec2& total = pushes[mover.owner];
		if (std::abs(push.x) > std::abs(total.x)) total.x = push.x;
		if (std::abs(push.y) > std::abs(total.y)) total.y = push.y;
	}

	for (auto& [agent, push] : pushes) {
		agent->GetTransform().position += push;
	}
}
