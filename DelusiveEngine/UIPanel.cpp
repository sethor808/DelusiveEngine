#include "UIPanel.h"
#include "Renderer.h"
#include "Shader.h"
#include <imgui/imgui.h>

UIPanel::UIPanel() 
    : UIElement("UIPanel", { 0,0 }), size({ 1,1 })
{
    name = "UIPanel";
}

UIPanel::UIPanel(const glm::vec2& pos, const glm::vec2& siz)
    : UIElement("UIPanel", pos), size(siz) {
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

void UIPanel::DrawImGui() {
    ImGui::Text("UIPanel");
    ImGui::DragFloat2("Position", &position.x, 1.0f);
    ImGui::DragFloat2("Size", &size.x, 1.0f);
    ImGui::ColorEdit4("Color", &color.x);
}

void UIPanel::Serialize(std::ostream& out) const {
    out << "UIPanel " << name << "\n";
    out << "pos " << position.x << " " << position.y << "\n";
    out << "size " << size.x << " " << size.y << "\n";
    out << "color " << color.r << " " << color.g << " " << color.b << " " << color.a << "\n";
    out << "---\n";
}

void UIPanel::Deserialize(std::istream& in) {
    std::string token;
    while (in >> token) {
        if (token == "---") break;
        else if (token == "pos") in >> position.x >> position.y;
        else if (token == "size") in >> size.x >> size.y;
        else if (token == "color") in >> color.r >> color.g >> color.b >> color.a;
    }
}

const std::string& UIPanel::GetTypeName() const {
    static std::string type = "UIPanel";
    return type;
}