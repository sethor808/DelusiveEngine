#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "ColliderComponent.h"
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

class PhysicsSystem {
public:
	static void HandleCollisions(const std::vector<std::unique_ptr<Agent>>&);
private:
	static bool CheckAABBCollision(ColliderComponent*, ColliderComponent*);
	static bool CheckBoxBoxCollision(ColliderComponent*, ColliderComponent*);
	static bool CheckCircleCircleCollision(ColliderComponent*, ColliderComponent*);
	static bool CheckBoxCircleCollision(ColliderComponent*, ColliderComponent*);
	static bool CheckLineLineCollision(ColliderComponent*, ColliderComponent*);
	static bool CheckLineCircleCollision(ColliderComponent*, ColliderComponent*);
	static bool CheckLineBoxCollision(ColliderComponent*, ColliderComponent*);
	static void ResolveSolidCollision(ColliderComponent*, ColliderComponent*);
};