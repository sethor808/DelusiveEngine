#pragma once
#include "Component.h"
#include "TransformData.h"
#include "ColliderRenderer.h"
#include "Agent.h"
#include <glm/glm.hpp>

enum class ColliderType {
	Solid,
	Hitbox,
	Hurtbox,
	Trigger
};

enum class ColliderHandleType {
    None,
    Center,
    Top, Bottom, Left, Right,
    TopLeft, TopRight, BottomLeft, BottomRight
};

enum class ShapeType {
	Box,
	Circle,
	Line
};

enum class ColliderAction {
	None,
	Drag,
	ResizeLeft,
	ResizeRight,
	ResizeTop,
	ResizeBottom
};

class Component;
class Agent;
class ColliderRenderer;

class ColliderComponent : public Component{
public:
	TransformComponent transform;
	ColliderComponent();
	virtual ~ColliderComponent() = default;
	std::unique_ptr<Component> Clone() const override = 0;

	virtual ColliderType GetColliderType() const = 0;
	virtual ShapeType GetShapeType() const = 0;
	virtual bool CheckCenterRender() const { return showCenter; }
	virtual void ToggleCenterDisplay() { showCenter = !showCenter; };
	
	void Draw(const glm::mat4& projection) const override;
	void HandleMouse(const glm::vec2&, bool) override;

	static void SetRenderer(ColliderRenderer* r) { renderer = r; }

	virtual void OnCollision(ColliderComponent* other) = 0;
	ColliderAction FromColliderHandleType(ColliderHandleType h);
protected:
	bool showCenter = false;
	static ColliderRenderer* renderer;
	ColliderHandleType activeHandle = ColliderHandleType::None;
	ColliderAction currentAction = ColliderAction::None;
	glm::vec2 dragStartMouse;
	glm::vec2 dragStartPos;
	glm::vec2 dragStartSize;
};