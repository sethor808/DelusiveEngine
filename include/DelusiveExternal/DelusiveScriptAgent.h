#pragma once
#include <cstdint>
#include <DelusiveExternal/Transform.h>
#include <string>

class Agent; //Forward declaration

class DelusiveScriptAgent {
public:
	Transform* transform;

	DelusiveScriptAgent(Agent* engineAgent);

	uint64_t GetID() const;
	const std::string& GetName() const;
private:
	Agent* agent; //Non-owned, points to engine side agent
};