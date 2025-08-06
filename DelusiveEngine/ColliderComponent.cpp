#include "ColliderComponent.h"
#include <iostream>
ColliderRenderer* ColliderComponent::renderer = nullptr;

template<typename T>
const T& Clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

ColliderComponent::ColliderComponent(){ 
	ColliderComponent::SetRenderer(renderer);
    char newName[64] = "New Collider";
    this->SetName(newName);
}

glm::vec2 ColliderComponent::GetMin() const {
    return transform.position;
}

glm::vec2 ColliderComponent::GetMax() const {
    return transform.position + transform.scale;
}

void ColliderComponent::Draw(const glm::mat4& projection) const{
	//std::cout << "[ColliderComponent] Draw called" << std::endl;
	renderer->Draw(*this, projection);
}

void ColliderComponent::HandleMouse(const glm::vec2& worldMouse, bool mouseDown) {
    glm::mat4 world = GetOwner()->GetTransform().GetTransformMatrix();
    glm::vec2 center = transform.position;
    float radius = transform.scale.x * 0.5f; // assuming uniform scaling

    float handleRadius = 6.0f / (64.0f * 1); // TODO: replace with dynamic zoom scaling

    if (GetShapeType() == ShapeType::Circle) {
        glm::vec2 worldCenter = glm::vec2(world * glm::vec4(center, 0, 1));
        glm::vec2 worldRightHandle = glm::vec2(world * glm::vec4(center + glm::vec2(radius, 0), 0, 1));

        if (!mouseDown && currentAction != ColliderAction::None) {
            currentAction = ColliderAction::None;
            return;
        }

        if (mouseDown && currentAction == ColliderAction::None) {
            if (glm::length(worldMouse - worldCenter) < handleRadius) {
                currentAction = ColliderAction::Drag;
                dragStartMouse = worldMouse;
                return;
            }
            else if (glm::length(worldMouse - worldRightHandle) < handleRadius) {
                currentAction = ColliderAction::ResizeRight;
                dragStartMouse = worldMouse;
                return;
            }
        }

        if (mouseDown && currentAction != ColliderAction::None) {
            glm::vec2 delta = worldMouse - dragStartMouse;
            dragStartMouse = worldMouse;

            switch (currentAction) {
            case ColliderAction::Drag:
                transform.position += delta;
                break;
            case ColliderAction::ResizeRight: {
                glm::vec2 localDelta = glm::inverse(world) * glm::vec4(delta, 0, 0);
                float newRadius = glm::length(localDelta + glm::vec2(radius, 0));
                transform.scale.x = transform.scale.y = newRadius * 2.0f;
                break;
            }
            default:
                break;
            }
        }

        return; // Circle done, skip box logic
    }

    if (GetShapeType() == ShapeType::Line) {
        glm::vec2 start = transform.position;
        float length = transform.scale.x;
        float angle = transform.rotation;
        glm::vec2 dir = glm::vec2(cos(angle), sin(angle));
        glm::vec2 end = start + dir * length;

        glm::mat4 world = GetOwner()->GetTransform().GetTransformMatrix();
        glm::vec2 worldStart = glm::vec2(world * glm::vec4(start, 0, 1));
        glm::vec2 worldEnd = glm::vec2(world * glm::vec4(end, 0, 1));
        glm::vec2 worldCenter = (worldStart + worldEnd) * 0.5f;

        float handleRadius = 6.0f / (64.0f * 1); // Zoom-scaled handle size

        if (!mouseDown && currentAction != ColliderAction::None) {
            currentAction = ColliderAction::None;
            return;
        }

        if (mouseDown && currentAction == ColliderAction::None) {
            if (glm::length(worldMouse - worldStart) < handleRadius) {
                currentAction = ColliderAction::ResizeLeft; // Start point drag
                dragStartMouse = worldMouse;
                return;
            }
            else if (glm::length(worldMouse - worldEnd) < handleRadius) {
                currentAction = ColliderAction::ResizeRight; // End point drag
                dragStartMouse = worldMouse;
                return;
            }
            else if (glm::length(worldMouse - worldCenter) < handleRadius) {
                currentAction = ColliderAction::Drag; // Move the line
                dragStartMouse = worldMouse;
                return;
            }
        }

        if (mouseDown && currentAction != ColliderAction::None) {
            glm::vec2 delta = worldMouse - dragStartMouse;
            dragStartMouse = worldMouse;

            glm::mat4 inverseWorld = glm::inverse(world);

            switch (currentAction) {
            case ColliderAction::Drag:
                transform.position += delta;
                break;

            case ColliderAction::ResizeLeft: {
                // Dragging start point
                glm::vec2 newStart = glm::vec2(world * glm::vec4(transform.position, 0, 1)) + delta;
                glm::vec2 newEnd = worldEnd;
                glm::vec2 newVec = newEnd - newStart;

                transform.position = glm::vec2(inverseWorld * glm::vec4(newStart, 0, 1));
                transform.rotation = atan2(newVec.y, newVec.x);
                transform.scale.x = glm::length(newVec);
                break;
            }

            case ColliderAction::ResizeRight: {
                // Dragging end point
                glm::vec2 newEnd = worldEnd + delta;
                glm::vec2 newVec = newEnd - worldStart;

                transform.rotation = atan2(newVec.y, newVec.x);
                transform.scale.x = glm::length(newVec);
                // Start stays the same (transform.position)
                break;
            }

            default:
                break;
            }
        }

        return;
    }

    // --- Default Box collider logic follows ---
    glm::vec2 halfSize = transform.scale * 0.5f;

    std::vector<std::pair<ColliderHandleType, glm::vec2>> handles = {
        { ColliderHandleType::Center,       glm::vec2(world * glm::vec4(center, 0, 1)) },
        { ColliderHandleType::Left,         glm::vec2(world * glm::vec4(center.x - halfSize.x, center.y, 0, 1)) },
        { ColliderHandleType::Right,        glm::vec2(world * glm::vec4(center.x + halfSize.x, center.y, 0, 1)) },
        { ColliderHandleType::Top,          glm::vec2(world * glm::vec4(center.x, center.y + halfSize.y, 0, 1)) },
        { ColliderHandleType::Bottom,       glm::vec2(world * glm::vec4(center.x, center.y - halfSize.y, 0, 1)) },
        { ColliderHandleType::TopLeft,      glm::vec2(world * glm::vec4(center - halfSize, 0, 1)) },
        { ColliderHandleType::TopRight,     glm::vec2(world * glm::vec4(center + glm::vec2(halfSize.x, -halfSize.y), 0, 1)) },
        { ColliderHandleType::BottomLeft,   glm::vec2(world * glm::vec4(center + glm::vec2(-halfSize.x, halfSize.y), 0, 1)) },
        { ColliderHandleType::BottomRight,  glm::vec2(world * glm::vec4(center + halfSize, 0, 1)) }
    };

    if (!mouseDown && currentAction != ColliderAction::None) {
        currentAction = ColliderAction::None;
        return;
    }

    if (mouseDown && currentAction == ColliderAction::None) {
        for (auto& [type, pos] : handles) {
            if (glm::length(worldMouse - pos) < handleRadius) {
                currentAction = ColliderComponent::FromColliderHandleType(type);
                dragStartMouse = worldMouse;
                return;
            }
        }
    }

    if (mouseDown && currentAction != ColliderAction::None) {
        glm::vec2 delta = worldMouse - dragStartMouse;
        dragStartMouse = worldMouse;

        switch (currentAction) {
        case ColliderAction::Drag:
            transform.position += delta;
            break;
        case ColliderAction::ResizeRight:
            transform.scale.x += delta.x;
            break;
        case ColliderAction::ResizeLeft:
            transform.position.x += delta.x;
            transform.scale.x -= delta.x;
            break;
        case ColliderAction::ResizeTop:
            transform.scale.y += delta.y;
            break;
        case ColliderAction::ResizeBottom:
            transform.position.y += delta.y;
            transform.scale.y -= delta.y;
            break;
        default:
            break;
        }
    }
}

ColliderAction ColliderComponent::FromColliderHandleType(ColliderHandleType h) {
    switch (h) {
    case ColliderHandleType::Center:
        return ColliderAction::Drag;
        break;
    case ColliderHandleType::Top:
        return ColliderAction::ResizeTop;
        break;
    case ColliderHandleType::Bottom:
        return ColliderAction::ResizeBottom;
        break;
    case ColliderHandleType::Left:
        return ColliderAction::ResizeLeft;
        break;
    case ColliderHandleType::Right:
        return ColliderAction::ResizeRight;
        break;
    default:
        return ColliderAction::None;
    }
}