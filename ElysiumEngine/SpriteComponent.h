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

enum class SpriteAction {
    None, Drag, ResizeTop, ResizeBottom, ResizeLeft, ResizeRight,
    ResizeTopLeft, ResizeTopRight, ResizeBottomLeft, ResizeBottomRight
};

struct SpriteInteractionState {
    SpriteAction currentAction = SpriteAction::None;
    glm::vec2 dragOffset = {};
    bool isSelected = false;
};

class SpriteComponent : public Component {
public:
    SpriteComponent(const char* texturePath);
    ~SpriteComponent();
    std::unique_ptr<Component> Clone() const override;

    void SetPosition(float x, float y);
    void SetScale(float sx, float sy);
    void SetRotation(float angle);
    void Draw(const glm::mat4& projection) const override;
    void DrawImGui() override;
    void SetVelocity(float x, float y);
    void Update(float) override;
    void SetLocalTransform(const glm::vec2&, const glm::vec2&, float) override;
    void HandleMouse(const glm::vec2&, bool) override;

    const char* GetType() const override {
        return "SpriteComponent";
    }

    void Serialize(std::ofstream& out) const override;
    void Deserialize(std::ifstream& in) override;
private:
    std::string texturePath;
    SpriteInteractionState interaction;
    GLuint VAO, VBO;
    Shader* shader;
    Texture* texture;
    glm::vec2 velocity = { 0.0f, 0.0f };
};
