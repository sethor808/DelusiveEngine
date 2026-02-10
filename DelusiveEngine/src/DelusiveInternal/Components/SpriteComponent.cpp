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

     0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.0f, 0.0f
};

SpriteComponent::SpriteComponent(DelusiveRenderer& renderer)
    : Component(renderer)
{
    Init();
    RegisterProperties();
}

void SpriteComponent::Init() {
    this->SetName("New Sprite");

    stbi_set_flip_vertically_on_load(true);

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

std::unique_ptr<Component> SpriteComponent::Clone() const {
    auto sprite = std::make_unique<SpriteComponent>(renderer);
    sprite->textureData.texturePath = textureData.texturePath;
	sprite->textureData.textureID = textureData.textureID;
    sprite->SetPosition(transform->position.x, transform->position.y);
    sprite->SetRotation(transform->rotation);
    sprite->SetScale(transform->scale.x, transform->scale.y);
    sprite->SetName(GetName());
    return sprite;
}

void SpriteComponent::SetTexturePath(const std::string& path) {
    textureData.texturePath = path;
    textureData.textureID = renderer.GetTexture(path);
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

    renderer.Submit({
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

bool SpriteComponent::DrawAnimatorImGui(ComponentMod& mod) {
    bool dirty = false;
    ImGui::Checkbox("Enabled", &enabled);
    dirty |= ImGui::IsItemEdited();

    dirty |= ImGui::DragFloat2("Offset", glm::value_ptr(transform->position), 1.0f);
    dirty |= ImGui::DragFloat2("Scale", glm::value_ptr(transform->scale), 0.01f);
    dirty |= ImGui::DragFloat("Rotation", &transform->rotation, 0.01f);

    ImGui::Text("Texture: %s", std::filesystem::path(textureData.texturePath).filename().string().c_str());
    if (ImGui::Button("Change Texture")) {
        ImGui::OpenPopup("TextureBrowser");
    }

    if (ImGui::BeginPopup("TextureBrowser")) {
        std::function<void(const std::filesystem::path&)> DrawDirectory;
        DrawDirectory = [&](const std::filesystem::path& path) {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (entry.is_directory()) {
                    if (ImGui::BeginMenu((entry.path().filename().string() + "/").c_str())) {
                        DrawDirectory(entry.path()); // recursive submenu
                        ImGui::EndMenu();
                    }
                }
                else if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                        std::string fullPath = entry.path().string();
                        std::string filename = entry.path().filename().string();
                        if (ImGui::Selectable(filename.c_str())) {
                            SetTexturePath(fullPath);
                            dirty = true;
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
            }
        };

        DrawDirectory(SPRITE_FOLDER);

        ImGui::EndPopup();
    }

    if (dirty) {
        mod.enabled = enabled;
        mod.positionOffset = transform->position;
        mod.scale = transform->scale;
        mod.rotation = transform->rotation;
        mod.texturePath = textureData.texturePath;
    }

    return dirty;
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

void SpriteComponent::Deserialize(std::istream& in) {
    Component::Deserialize(in);

    SetTexturePath(textureData.texturePath);
}
