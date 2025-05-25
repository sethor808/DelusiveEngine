#pragma once
#include "Component.h"
#include "Shader.h"
#include "Texture.h"
#include "TransformData.h"
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <imgui/imgui.h>
#include <imgui/backend/imgui_impl_sdl3.h>
#include <imgui/backend/imgui_impl_opengl3.h>

class SpriteComponent : public Component {
public:
    SpriteComponent(const char* texturePath);
    ~SpriteComponent();
    void SetPosition(float x, float y);
    void SetScale(float sx, float sy);
    void SetRotation(float angle);
    void Draw(const glm::mat4& projection) const override;
    void DrawImGui() override;
    void SetVelocity(float x, float y);
    void Update(float) override;

    const char* GetType() const override {
        return "Sprite";
    }

private:
    TransformComponent transform;
    std::string texturePath;
    GLuint VAO, VBO;
    Shader* shader;
    Texture* texture;
    glm::vec2 velocity = { 0.0f, 0.0f };
};
