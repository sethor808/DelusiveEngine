#include <Delusive/Runtime/Components/ColliderComponent.h>
#include <Delusive/Runtime/Core/PhysicsSystem.h>
#include <algorithm>
#include <cmath>
#include <iostream>

ColliderComponent::ColliderComponent(DelusiveInstance& instance)
    : Component(instance)
{ 
	name = "New Collider";
    //Concrete colliders call RegisterProperties - GetType() is still pure here
}

void ColliderComponent::RegisterProperties() {
    Component::RegisterProperties();
    registry->Register("shape", reinterpret_cast<int*>(&shape));
}

void ColliderComponent::Draw(const ColliderRenderer& renderer, const glm::mat4& projection) const{
	//std::cout << "[ColliderComponent] Draw called" << std::endl;
    if (editorMode) {
        renderer.Draw(*this, projection);
    }
}

std::vector<ColliderComponent::Handle> ColliderComponent::GetHandles() const {
    const WorldShape s = PhysicsSystem::BuildShape(*this);
    using H = ColliderHandleType;

    switch (s.shape) {
    case ShapeType::Circle:
        return { { H::Center, s.center }, { H::Right, s.center + glm::vec2(s.radius, 0.0f) } };
    case ShapeType::Line:
        //Left is the start point, Right the end
        return { { H::Left, s.center }, { H::Right, s.end }, { H::Center, (s.center + s.end) * 0.5f } };
    case ShapeType::Box:
    default: {
        const glm::vec2 c = (s.min + s.max) * 0.5f;
        return {
            { H::Center, c },
            { H::Left, { s.min.x, c.y } },        { H::Right, { s.max.x, c.y } },
            { H::Bottom, { c.x, s.min.y } },      { H::Top, { c.x, s.max.y } },
            { H::BottomLeft, s.min },             { H::BottomRight, { s.max.x, s.min.y } },
            { H::TopLeft, { s.min.x, s.max.y } }, { H::TopRight, s.max },
        };
    }
    }
}

void ColliderComponent::HandleMouse(const glm::vec2& worldMouse, bool mouseDown) {
    const bool pressed = mouseDown && !mouseWasDown;
    mouseWasDown = mouseDown;
    if (!editorMode) return;

    if (!mouseDown) {
        activeHandle = ColliderHandleType::None;
        return;
    }

    if (pressed) {
        //Nearest handle under the cursor
        float best = HandleSize * 0.5f;
        for (const Handle& handle : GetHandles()) {
            const float distance = glm::length(worldMouse - handle.position);
            if (distance <= best) {
                best = distance;
                activeHandle = handle.type;
            }
        }
        dragStartMouse = worldMouse;
        return;
    }

    if (activeHandle == ColliderHandleType::None) return;

    //Edits happen in world space, then convert back through the agent:
    //world = agent position + agent scale * local
    const Transform& agent = GetOwner()->GetTransform();
    auto Safe = [](float v) { return std::abs(v) < 1e-6f ? (v < 0.0f ? -1e-6f : 1e-6f) : v; };
    const glm::vec2 agentScale = { Safe(agent.scale.x), Safe(agent.scale.y) };
    auto ToLocalPoint = [&](glm::vec2 world) { return (world - agent.position) / agentScale; };

    const WorldShape s = PhysicsSystem::BuildShape(*this);
    const glm::vec2 delta = worldMouse - dragStartMouse;
    dragStartMouse = worldMouse;
    const float minSize = 0.01f;
    using H = ColliderHandleType;

    switch (s.shape) {
    case ShapeType::Circle: {
        if (activeHandle == H::Center) {
            transform->position = ToLocalPoint(s.center + delta);
        }
        else {
            const float radius = std::max(glm::length(worldMouse - s.center), minSize);
            const float agentReach = std::max(std::abs(agentScale.x), std::abs(agentScale.y));
            transform->scale.x = transform->scale.y = 2.0f * radius / agentReach;
        }
        break;
    }
    case ShapeType::Line: {
        glm::vec2 start = s.center, end = s.end;
        if (activeHandle == H::Left)        start = worldMouse;
        else if (activeHandle == H::Right)  end = worldMouse;
        else { start += delta; end += delta; }

        const glm::vec2 localVec = (end - start) / agentScale;
        transform->position = ToLocalPoint(start);
        transform->rotation = std::atan2(localVec.y, localVec.x);
        transform->scale.x = glm::length(localVec);
        break;
    }
    case ShapeType::Box:
    default: {
        glm::vec2 mn = s.min, mx = s.max;
        const bool left = activeHandle == H::Left || activeHandle == H::TopLeft || activeHandle == H::BottomLeft;
        const bool right = activeHandle == H::Right || activeHandle == H::TopRight || activeHandle == H::BottomRight;
        const bool bottom = activeHandle == H::Bottom || activeHandle == H::BottomLeft || activeHandle == H::BottomRight;
        const bool top = activeHandle == H::Top || activeHandle == H::TopLeft || activeHandle == H::TopRight;

        //The grabbed edge follows the mouse; the opposite edge stays put
        if (activeHandle == H::Center) { mn += delta; mx += delta; }
        if (left)   mn.x = std::min(worldMouse.x, mx.x - minSize);
        if (right)  mx.x = std::max(worldMouse.x, mn.x + minSize);
        if (bottom) mn.y = std::min(worldMouse.y, mx.y - minSize);
        if (top)    mx.y = std::max(worldMouse.y, mn.y + minSize);

        transform->position = ToLocalPoint((mn + mx) * 0.5f);
        transform->scale = (mx - mn) / glm::abs(agentScale);
        break;
    }
    }
}
