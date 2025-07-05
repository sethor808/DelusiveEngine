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

void ColliderComponent::Draw(const glm::mat4& projection) const{
	std::cout << "[ColliderComponent] Draw called" << std::endl;
	renderer->Draw(*this, projection);
}

void ColliderComponent::HandleMouse(const glm::vec2& worldMouse, bool mouseDown) {
    glm::mat4 world = GetOwner()->GetTransform().GetTransformMatrix();
    glm::vec2 center = transform.position;
    glm::vec2 halfSize = transform.scale * 0.5f;

    // World-space handle positions
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

    float handleRadius = 6.0f / (64.0 * 1); //TODO: Pull the zoom from the main camera

    std::cout << "Mouse world: " << worldMouse.x << ", " << worldMouse.y << "\n";
    std::cout << "Center handle: " << handles[0].second.x << ", " << handles[0].second.y << "\n";

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
            // TODO: handle corners
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