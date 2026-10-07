#include <Delusive/Runtime/Components/SpriteComponent.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Runtime/Components/TransformComponent.h>
#include <Delusive/Internal/Rendering/TextureManager.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <string>
#include <imgui/imgui.h>
#include <imgui/backend/imgui_impl_sdl3.h>
#include <imgui/backend/imgui_impl_opengl3.h>

float vertices[] = {
    // pos       // tex
    -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  1.0f, 0.0f,


     0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.0f, 0.0f
};

SpriteComponent::SpriteComponent(DelusiveInstance& instance)
    : Component(instance)
{
    Init();
    RegisterProperties();
}

void SpriteComponent::Init() {
    this->SetName("New Sprite");

    SetPosition(0.0f, 0.0f);
    SetRotation(0.0f);
    SetScale(1.0f, 1.0f);

    if (!textureData.texturePath.empty()) {
        SetTexturePath(textureData.texturePath);
    }
}

void SpriteComponent::RegisterProperties() {
    Component::RegisterProperties();
    registry->Register("textureData", &textureData);
}

void SpriteComponent::Deserialize(DelusiveParser::DataBlock& in) {
    Component::Deserialize(in);
    SetTexturePath(textureData.texturePath);
}

void SpriteComponent::SetTexturePath(const std::string& path) {
    textureData.texturePath = path;
    textureData.textureID = instance.renderer.GetTexture(path);
}

void SpriteComponent::SetPosition(float x, float y) {
    transform->position = { x, y };
}

void SpriteComponent::SetScale(float sx, float sy) {
    transform->scale = { sx, sy };
}

void SpriteComponent::SetRotation(float angle) {
    transform->rotation = { angle };
}

void SpriteComponent::Draw(const glm::mat4& projection) const{
    glm::mat4 agentTransform = owner->GetTransform().ToMatrix();
    glm::mat4 localTransform = transform->ToMatrix();
    glm::mat4 model = agentTransform * localTransform;

    instance.renderer.Submit({
        .modelMatrix = model,
        .color = glm::vec4(1.0f), // could expose as property
        .textureID = textureData.textureID,
        .layer = renderOrder,
        .isUI = false
        });
}

void SpriteComponent::DrawImGui() {
    Component::DrawImGui();

    SetTexturePath(textureData.texturePath);
}

void SpriteComponent::SetVelocity(float x, float y) {
    velocity = { x, y };
}

void SpriteComponent::Update(float deltaTime){
    transform->position += velocity * deltaTime;
    //Probably move camera here
}

void SpriteComponent::SetLocalTransform(const glm::vec2& pos, const glm::vec2& scale, float rot) {
    transform->position = pos;
    transform->scale = scale;
    transform->rotation = rot;
}

void SpriteComponent::HandleMouse(const glm::vec2& worldMouse, bool isMouseDown) {
    if (!enabled) return;

    if (editorMode) {
        glm::vec2 center = owner->GetTransform().position + transform->position;
        glm::vec2 halfSize = transform->scale * 0.5f;

        glm::vec2 min = center - halfSize;
        glm::vec2 max = center + halfSize;

        bool mouseOver = worldMouse.x >= min.x && worldMouse.x <= max.x &&
            worldMouse.y >= min.y && worldMouse.y <= max.y;

        if (!isMouseDown && interaction.currentAction == EditorAction::None) {
            interaction.isSelected = mouseOver;
        }

        if (isMouseDown && interaction.currentAction == EditorAction::None && mouseOver) {
            interaction.currentAction = EditorAction::Drag;
            interaction.dragOffset = (worldMouse - center) / transform->scale;
        }

        if (!isMouseDown) {
            interaction.currentAction = EditorAction::None;
        }

        if (interaction.currentAction == EditorAction::Drag) {
            glm::vec2 delta = (worldMouse - owner->GetTransform().position) - (interaction.dragOffset * transform->scale);
            transform->position = delta;
        }
    }
}