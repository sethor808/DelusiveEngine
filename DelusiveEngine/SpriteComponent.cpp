#include "SpriteComponent.h"
#include "Agent.h"
#include "DelusiveMacros.h"
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <string>

float vertices[] = {
    // pos       // tex
    -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  1.0f, 1.0f,

     0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.0f, 0.0f
};

SpriteComponent::SpriteComponent() {
    Init();
}

SpriteComponent::SpriteComponent(const char* texturePath) {
    textureData.texturePath = texturePath;
    Init();
    RegisterProperties();
}

void SpriteComponent::Init() {
    this->SetName("New Sprite");

    stbi_set_flip_vertically_on_load(true);

    SetPosition(0.0f, 0.0f);
    SetRotation(0.0f);
    SetScale(1.0f, 1.0f);

    textureData.Init(); // VAO/VBO/Shader setup

    if (!textureData.texturePath.empty()) {
        textureData.SetTexture(textureData.texturePath);
    }
}

void SpriteComponent::RegisterProperties() {
    Component::RegisterProperties();
    registry->Register("textureData", &textureData);
}

std::unique_ptr<Component> SpriteComponent::Clone() const {
    auto sprite = std::make_unique<SpriteComponent>(textureData.texturePath.c_str());
    sprite->SetPosition(transform.position.x, transform.position.y);
    sprite->SetRotation(transform.rotation);
    sprite->SetScale(transform.scale.x, transform.scale.y);
    sprite->SetName(GetName());
    return sprite;
}

void SpriteComponent::SetTexturePath(const std::string& path) {
    // Don't reload if it's the same texture
    //if (texturePath == path) return; //Commented out because of how the new property registry works

    textureData.texturePath = path;

    // Clean up old texture
    if (textureData.texture) {
        delete textureData.texture;
        textureData.texture = nullptr;
    }

    // Load new texture
    textureData.texture = new Texture(path.c_str());

    //TODO: Perhaps change this to load the previous texture if it doesn't load
    if (!textureData.texture) {
        std::cerr << "[SpriteComponent] Failed to load texture: " << path << "\n";
    }
    else {
        std::cout << "[SpriteComponent] Texture set to: " << path << "\n";
    }
}

void SpriteComponent::SetPosition(float x, float y) {
    transform.position = { x, y };
}

void SpriteComponent::SetScale(float sx, float sy) {
    transform.scale = { sx, sy };
}

void SpriteComponent::SetRotation(float angle) {
    transform.rotation = { angle };
}

void SpriteComponent::Draw(const glm::mat4& projection) const{
    glm::mat4 agentTransform = owner->GetTransform().ToMatrix();
    glm::mat4 localTransform = transform.ToMatrix();
    glm::mat4 model = agentTransform * localTransform;
    glm::mat4 view = glm::mat4(1.0f);

    textureData.Draw(model, view, projection);
}

void SpriteComponent::DrawImGui() {
    Component::DrawImGui();

	//Reloading the texture here to bypass Update not being called in the editor
    if (textureData.texturePath != textureData.previousTexturePath) {
        SetTexturePath(textureData.texturePath);
        textureData.previousTexturePath = textureData.texturePath;
    }
}

bool SpriteComponent::DrawAnimatorImGui(ComponentMod& mod) {
    bool dirty = false;
    ImGui::Checkbox("Enabled", &enabled);
    dirty |= ImGui::IsItemEdited();

    dirty |= ImGui::DragFloat2("Offset", glm::value_ptr(transform.position), 1.0f);
    dirty |= ImGui::DragFloat2("Scale", glm::value_ptr(transform.scale), 0.01f);
    dirty |= ImGui::DragFloat("Rotation", &transform.rotation, 0.01f);

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
        mod.positionOffset = transform.position;
        mod.scale = transform.scale;
        mod.rotation = transform.rotation;
        mod.texturePath = textureData.texturePath;
    }

    return dirty;
}

void SpriteComponent::SetVelocity(float x, float y) {
    velocity = { x, y };
}

void SpriteComponent::Update(float deltaTime){
    if (!textureData.texture) {
        if (textureData.texturePath != "") {
            SetTexturePath(textureData.texturePath);
        }
    }

    //Reload texture if changed
    if (textureData.texturePath != textureData.previousTexturePath) {
        SetTexturePath(textureData.texturePath);
        textureData.previousTexturePath = textureData.texturePath;
    }

    transform.position += velocity * deltaTime;
    //Probably move camera here
}

void SpriteComponent::SetLocalTransform(const glm::vec2& pos, const glm::vec2& scale, float rot) {
    transform.position = pos;
    transform.scale = scale;
    transform.rotation = rot;
}

void SpriteComponent::HandleMouse(const glm::vec2& worldMouse, bool isMouseDown) {
    if (!enabled) return;

    if (editorMode) {
        glm::vec2 center = owner->GetTransform().position + transform.position;
        glm::vec2 halfSize = transform.scale * 0.5f;

        glm::vec2 min = center - halfSize;
        glm::vec2 max = center + halfSize;

        bool mouseOver = worldMouse.x >= min.x && worldMouse.x <= max.x &&
            worldMouse.y >= min.y && worldMouse.y <= max.y;

        if (!isMouseDown && interaction.currentAction == EditorAction::None) {
            interaction.isSelected = mouseOver;
        }

        if (isMouseDown && interaction.currentAction == EditorAction::None && mouseOver) {
            interaction.currentAction = EditorAction::Drag;
            interaction.dragOffset = (worldMouse - center) / transform.scale;
        }

        if (!isMouseDown) {
            interaction.currentAction = EditorAction::None;
        }

        if (interaction.currentAction == EditorAction::Drag) {
            glm::vec2 delta = (worldMouse - owner->GetTransform().position) - (interaction.dragOffset * transform.scale);
            transform.position = delta;
        }
    }
}

void SpriteComponent::Deserialize(std::istream& in) {
    Component::Deserialize(in);

    SetTexturePath(textureData.texturePath);
}
