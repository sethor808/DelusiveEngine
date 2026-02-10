#include <DelusiveExternal/ScriptUIElement.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <DelusiveExternal/DelusiveUICanvas.h>

DelusiveUICanvas ScriptUIElement::GetCanvas() {
    DelusiveUICanvas canvasLink;
    canvasLink.Link(element->GetCanvas());
    return canvasLink;
}

bool ScriptUIElement::SupportsClick() {
    return IsValid() ? element->SupportsClick() : false;
}

std::string ScriptUIElement::GetType() const {
    return IsValid() ? element->GetType() : "";
}

std::string ScriptUIElement::GetName() const {
    return IsValid() ? element->GetName() : "";
}

bool ScriptUIElement::IsEnabled() const {
    return IsValid() ? element->GetEnabled() : false;
}

void ScriptUIElement::SetEnabled(bool enabled) {
    if (element) element->SetEnabled(enabled);
}

glm::vec2 ScriptUIElement::GetPosition() const {
    return IsValid() ? element->GetPosition() : glm::vec2{};
}

void ScriptUIElement::SetPosition(const glm::vec2& pos) {
    if (element) element->SetPosition(pos);
}

glm::vec2 ScriptUIElement::GetSize() const {
    return element ? element->GetSize() : glm::vec2{};
}

void ScriptUIElement::SetSize(const glm::vec2& size) {
    if (element) element->SetSize(size);
}

std::vector<ScriptUIElement> ScriptUIElement::GetChildren() const {
    std::vector<ScriptUIElement> out;
    if (!element) return out;

    for (UIElement* child : element->GetChildren()) {
        ScriptUIElement toAdd;
        toAdd.Link(child);
        out.emplace_back(toAdd);
    }
    return out;
}