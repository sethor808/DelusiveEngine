#pragma once
#include "Component.h"
#include "TransformData.h"
#include "ColliderRenderer.h"
#include <glm/glm.hpp>

enum class ColliderType {
	Solid,
	Hitbox,
	Hurtbox,
	Trigger
};

enum class ShapeType {
	Box,
	Circle,
	Line
};

class Component;
class ColliderRenderer;

class ColliderComponent : public Component{
public:
	TransformComponent transform;
	ColliderComponent();
	virtual ~ColliderComponent() = default;

	virtual ColliderType GetColliderType() const = 0;
	virtual ShapeType GetShapeType() const = 0;
	virtual bool CheckCenterRender() const { return showCenter; }
	virtual void ToggleCenterDisplay() { showCenter = !showCenter; };
	
	void Draw(const glm::mat4& projection) const override;

	static void SetRenderer(ColliderRenderer* r) { renderer = r; }

	virtual void OnCollision(ColliderComponent* other) = 0;
protected:
	bool showCenter = false;
	static ColliderRenderer* renderer;
};