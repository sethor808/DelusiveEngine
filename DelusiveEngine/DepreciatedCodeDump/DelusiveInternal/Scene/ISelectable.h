#pragma once
#include <glm/glm.hpp>

class ISelectable {
public:
	virtual ~ISelectable() = default;

	virtual bool HitTest(const glm::vec2& point) = 0;

	virtual void OnSelect() = 0;
	virtual void OnDeselect() = 0;

	virtual void DrawSelectionGizmo() = 0;

	virtual void ONDrag(const glm::vec2& delta) = 0;
};