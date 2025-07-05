#pragma once
#include "ElysiumUtils.h"
#include "TransformData.h"
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>
#include <sstream>

class Agent;

class Component {
public:
	TransformComponent transform;

	virtual ~Component() = default;
	virtual std::unique_ptr<Component> Clone() const = 0;

	virtual void Update(float) = 0;
	virtual void Draw(const glm::mat4& projection) const {};
	virtual void DrawImGui() {}
	virtual void SetLocalTransform(const glm::vec2&, const glm::vec2&, float) {}

	virtual const char* GetType() const = 0;
	virtual const char* GetName() const {return name;}
	virtual void SetName(const char*);

	//Mouse handler hook
	virtual void HandleMouse(const glm::vec2&, bool) {}
	virtual bool IsDragging() const { return isDragging; }

	void SetOwner(Agent* agent) { this->owner = agent; }
	Agent* GetOwner() const { return owner; }

	void SetEnabled(bool enabled) { this->enabled = enabled; }
	bool IsEnabled() const { return this->enabled; }

	// Save/Load
	virtual void Serialize(std::ofstream& out) const = 0;
	virtual void Deserialize(std::ifstream& in) = 0;
protected:
	Agent* owner = nullptr;
	char name[64];
	bool enabled = true;
	bool isDragging = false;
};