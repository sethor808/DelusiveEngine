#include <Delusive/Runtime/UI/UIPanel.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Internal/Rendering/Shader.h>
#include <imgui/imgui.h>

UIPanel::UIPanel(DelusiveInstance& instance)
    : UIElement(instance)
{
    name = "UIPanel";

    RegisterProperties();
}

void UIPanel::RegisterProperties() {
    UIElement::RegisterProperties();
    //texture and shader are GL handles set at runtime, not authored data
    registry->Register("color", &color);
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
    instance.renderer.DrawRect(position, size, color, projection, shader, texture);

    // Draw children
    for (auto& child : children) {
        child->Draw(projection);
    }
}

const std::string UIPanel::GetType() const {
    return "UIPanel";
}