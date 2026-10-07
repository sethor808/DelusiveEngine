#pragma once
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/Components/TransformComponent.h>
#include <Delusive/Internal/Rendering/ColliderRenderer.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <glm/glm.hpp>
#include <vector>

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

class Component;
class Agent;
class ColliderRenderer;

class ColliderComponent : public Component{
public:
	ColliderComponent(DelusiveInstance&);
	ColliderComponent() = delete;

	ColliderComponent(const ColliderComponent&) = delete;
	ColliderComponent& operator=(const ColliderComponent&) = delete;
	ColliderComponent(ColliderComponent&&) noexcept = default;
	ColliderComponent& operator=(ColliderComponent&&) noexcept = default;

	virtual ~ColliderComponent() = default;

	void RegisterProperties() override;

	virtual ColliderType GetColliderType() const = 0;
	virtual ShapeType GetShapeType() const { return shape; }
	void SetShapeType(ShapeType newShape) { shape = newShape; }

	virtual bool CheckCenterRender() const { return showCenter; }
	virtual void ToggleCenterDisplay() { showCenter = !showCenter; };
	
	virtual void Draw(const ColliderRenderer&, const glm::mat4& ) const;
	void HandleMouse(const glm::vec2&, bool) override;

	virtual void OnCollision(ColliderComponent* other) = 0;

	//Editor handles in world space, built from the same shape that collides and draws
	struct Handle {
		ColliderHandleType type;
		glm::vec2 position;
	};
	std::vector<Handle> GetHandles() const;
	bool IsDraggingHandle() const { return activeHandle != ColliderHandleType::None; }
	//World size of a drawn handle; the mouse grabs within half of it
	static constexpr float HandleSize = 12.0f / DELUSIVE_PIXEL_SCALE;

protected:
	ShapeType shape = ShapeType::Box;
	bool showCenter = false;
	ColliderHandleType activeHandle = ColliderHandleType::None;
	glm::vec2 dragStartMouse{ 0.0f };
	//Handles are only grabbed when the button goes down, not by passing over them mid-drag
	bool mouseWasDown = false;
};