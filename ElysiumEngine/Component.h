#pragma once
#include <glm/glm.hpp>
#include <string>

class Agent;

class Component {
public:
	virtual ~Component() = default;

	virtual void Update(float) = 0;
	virtual void Draw(const glm::mat4& projection) const {};
	virtual void DrawImGui() {}

	virtual const char* GetType() const = 0;
	virtual char* GetName();
	virtual void SetName(char*);

	void SetOwner(Agent* agent) { this->owner = agent; }
	Agent* GetOwner() const { return owner; }

	void SetEnabled(Agent* agent) { this->owner = agent; }
	bool IsEnabled() const { enabled; }
protected:
	Agent* owner = nullptr;
	char name[64];
	bool enabled = false;
};