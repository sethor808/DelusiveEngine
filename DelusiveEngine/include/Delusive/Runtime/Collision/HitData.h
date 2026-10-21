#pragma once
#include <glm/vec2.hpp>

class Agent;

struct HitData {
	Agent* attacker = nullptr;
	int damage = 1;
	glm::vec2 direction = { 0.0f, 0.0f };
	bool parryable = true;
};
