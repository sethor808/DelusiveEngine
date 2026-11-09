#pragma once
#include <glm/vec2.hpp>

class Agent;

struct HitData {
	Agent* attacker = nullptr;
	Agent* target = nullptr;
	int damage = 1;
	glm::vec2 direction = { 0.0f, 0.0f };
	//Where the boxes met, for hit effects
	glm::vec2 point = { 0.0f, 0.0f };
	//Seconds the world freezes when the hit lands
	float hitStop = 0.05f;
	bool parryable = true;
};
