#include "UIPanel.h"
#include "Renderer.h"
#include "Shader.h"
#include <imgui/imgui.h>

UIPanel::UIPanel() 
    : UIElement("UIPanel", { 0,0 })
{
    name = "UIPanel";
}

UIPanel::UIPanel(const glm::vec2& pos, const glm::vec2& siz)
    : UIElement("UIPanel", pos){
}

std::unique_ptr<UIElement> UIPanel::Clone() const {
    auto copy = std::make_unique<UIPanel>();
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
    Renderer::DrawRect(position, size, color, projection, shader, texture);

    // Draw children
    for (auto& child : children) {
        child->Draw(projection);
    }
}

const std::string UIPanel::GetType() const {
    return "UIPanel";
}