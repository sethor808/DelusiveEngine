#pragma once
#include "DelusiveData.h"
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
    bool isForeground = false;

    SpriteComponent();
    SpriteComponent(const char* texturePath);

    SpriteComponent(const SpriteComponent&) = delete;
    SpriteComponent& operator=(const SpriteComponent&) = delete;
    SpriteComponent(SpriteComponent&&) noexcept = default;
    SpriteComponent& operator=(SpriteComponent&&) noexcept = default;

    ~SpriteComponent();
    void Init();

    void RegisterProperties() override;
    std::unique_ptr<Component> Clone() const override;

    void SetTexturePath(const std::string&) override;
    void SetPosition(float x, float y);
    void SetScale(float sx, float sy);
    void SetRotation(float angle);
    void Draw(const glm::mat4& projection) const override;
    void DrawImGui() override;
    bool DrawAnimatorImGui(ComponentMod&) override;
    void SetVelocity(float x, float y);
    void Update(float) override;
    void SetLocalTransform(const glm::vec2&, const glm::vec2&, float) override;
    void HandleMouse(const glm::vec2&, bool) override;

    const char* GetType() const override {
        return "SpriteComponent";
    }

    //void Serialize(std::ofstream& out) const override;
    void Deserialize(std::ifstream& in) override;
private:
    SpriteInteractionState interaction;
    GLuint VAO, VBO;
    Shader* shader;
    Texture* texture;
	DelusiveTexture textureData;
    
    int renderOrder = 0;
    glm::vec2 velocity = { 0.0f, 0.0f };
};
