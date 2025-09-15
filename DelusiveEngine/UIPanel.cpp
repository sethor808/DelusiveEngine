#include "UIPanel.h"
#include "DelusiveRenderer.h"
#include "Shader.h"
#include <imgui/imgui.h>

UIPanel::UIPanel(DelusiveRenderer& _renderer)
    : UIElement(_renderer)
{
    name = "UIPanel";
}

std::unique_ptr<UIElement> UIPanel::Clone() const {
    auto copy = std::make_unique<UIPanel>(renderer);
    copy->SetPosition(position);
    copy->SetSize(size);
    copy->SetColor(color);
    copy->SetShader(shader);
    copy->SetTexture(texture);

    for (const auto& child : children) {
        copy->AddChild(std::move(child->Clone()));
    }
    return copy;
}

void UIPanel::SetTexture(GLuint tex) {
    texture = tex;
}

void UIPanel::SetShader(Shader* s) {
    shader = s;
}

void UIPanel::SetColor(const glm::vec4& c) {
    color = c;
}

void UIPanel::Draw(const glm::mat4& projection) {
    if (shader) shader->Use();
    renderer.DrawRect(position, size, color, projection, shader, texture);

    // Draw children
    for (auto& child : children) {
        child->Draw(projection);
    }
}

const std::string UIPanel::GetType() const {
    return "UIPanel";
}