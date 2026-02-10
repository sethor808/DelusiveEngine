#pragma once
#include <glm/glm.hpp>

enum class EditorAction {
    None, Drag, ResizeTop, ResizeBottom, ResizeLeft, ResizeRight,
    ResizeTopLeft, ResizeTopRight, ResizeBottomLeft, ResizeBottomRight
};

struct InteractionState {
    EditorAction currentAction = EditorAction::None;
    glm::vec2 dragOffset = {};
    bool isSelected = false;
};