#pragma once
#include "UIElement.h"
#include <glm/glm.hpp>
#include <GL/glew.h>

class Shader;

class UIPanel : public UIElement {
public:
    UIPanel();
    UIPanel(const glm::vec2& pos, const glm::vec2& size);
    std::unique_ptr<UIElement> Clone() const override;

    void SetTexture(GLuint texture);
    void SetShader(Shader* shader);
    void SetColor(const glm::vec4& color);

    void Draw(const glm::mat4& proj) override;
    void HandleMouse(const glm::vec2&, bool) override {}
    void DrawImGui() override;

    void Serialize(std::ostream& out) const override;
    void Deserialize(std::istream& in) override;

    const std::string& GetTypeName() const override;

private:
    glm::vec2 size;
    glm::vec4 color = { 0.1f, 0.1f, 0.1f, 0.85f };

    GLuint texture = 0;
    Shader* shader = nullptr;
};