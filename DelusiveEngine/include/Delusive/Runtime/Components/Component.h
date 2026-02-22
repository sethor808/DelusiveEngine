#pragma once
#include <Delusive/Runtime/Utils/DelusiveUtils.h>
#include <Delusive/Runtime/Animation/AnimatorData.h>
#include <Delusive/Runtime/Utils/UUID.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>

class Agent;
class PropertyRegistry;
class DelusiveRenderer;
struct TransformComponent;

class Component {
public:
	std::unique_ptr<TransformComponent> transform;

	Component(DelusiveRenderer&);
	Component() = delete;

	Component(const Component&) = delete;
	Component& operator=(const Component&) = delete;
	Component(Component&&) noexcept = default;
	Component& operator=(Component&&) noexcept = default;

	virtual ~Component();
	virtual std::unique_ptr<Component> Clone() const = 0;

	virtual void RegisterProperties();

	virtual void Update(float) = 0;
	virtual void Draw(const glm::mat4& projection) const {};
	virtual void DrawImGui();
	virtual bool DrawAnimatorImGui(ComponentMod&) { return false; }
	virtual void SetEditorMode(bool editor) { editorMode = editor; }
	virtual void SetLocalTransform(const glm::vec2&, const glm::vec2&, float) {}
	
	virtual const char* GetType() const = 0;
	virtual const std::string GetName() const {return name;}
	virtual void SetName(const std::string&);

	//Mouse handler hook
	virtual void HandleMouse(const glm::vec2&, bool) {}
	virtual bool IsDragging() const { return isDragging; }

	virtual void SetOwner(Agent* agent) { this->owner = agent; }
	Agent* GetOwner() const { return owner; }

	void SetEnabled(bool enabled) { this->enabled = enabled; }
	bool IsEnabled() const { return this->enabled; }

	virtual bool ToDelete() const { return toDelete; }
	void MarkToDelete() { toDelete = true; }

    const UUID GetID() const {return id; }
	void SetID(UUID id) { this->id = id; }

	// Save/Load
	virtual void Serialize(std::ostream& out) const;
	virtual void Deserialize(std::istream& in);
protected:
	DelusiveRenderer& renderer;
	std::unique_ptr<PropertyRegistry> registry;
	Agent* owner = nullptr;
	bool editorMode = false;
	std::string name;
	UUID id;
	bool enabled = true;
	bool isDragging = false;
	bool toDelete = false;
};