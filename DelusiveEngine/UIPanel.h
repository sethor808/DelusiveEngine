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

    const std::string GetType() const override;
private:
    glm::vec4 color = { 0.1f, 0.1f, 0.1f, 0.85f };

    GLuint texture = 0;
    Shader* shader = nullptr;
};