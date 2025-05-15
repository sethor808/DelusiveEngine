#pragma once
#include <glm/glm.hpp>

class Agent;

class Component {
public:
	virtual ~Component() = default;

	virtual void Update(float) = 0;
	virtual void Draw(const glm::mat4& projection) {};
	virtual void DrawImGui() {}

	virtual const char* GetName() const = 0;

	void SetOwner(Agent* agent) { this->owner = agent; }
	Agent* GetOwner() const { return owner; }

protected:
	Agent* owner = nullptr;
};