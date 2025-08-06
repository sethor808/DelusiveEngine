#include "SpriteComponent.h"
#include "Agent.h"
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

SpriteComponent::SpriteComponent(const char* texturePath) {
    char newName[64] = "New Sprite";
    this->SetName(newName);

    stbi_set_flip_vertically_on_load(true);

    //TODO: Clean this up later
    SetPosition(0.0f, 0.0f);
    SetRotation(0.0f);
    SetScale(1.0f, 1.0f);

    std::cout << "[Sprite Component] position: " << transform.position.x << std::endl;

    this->texturePath = texturePath;
    shader = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
    texture = new Texture(texturePath);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

SpriteComponent::~SpriteComponent() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    delete texture;
    delete shader;
    //glDeleteTextures(1, &texture);
}

std::unique_ptr<Component> SpriteComponent::Clone() const {
    auto sprite = std::make_unique<SpriteComponent>(texturePath.c_str());
    sprite->SetPosition(transform.position.x, transform.position.y);
    sprite->SetRotation(transform.rotation);
    sprite->SetScale(transform.scale.x, transform.scale.y);
    sprite->SetName(GetName());
    return sprite;
}

void SpriteComponent::SetTexture(const std::string& path) {
    // Don't reload if it's the same texture
    if (texturePath == path) return;

    texturePath = path;

    // Clean up old texture
    if (texture) {
        delete texture;
        texture = nullptr;
    }

    // Load new texture
    texture = new Texture(path.c_str());

    //TODO: Perhaps change this to load the previous texture if it doesn't load
    if (!texture) {
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
    if (!shader || !texture) {
        std::cerr << "[SpriteComponent::Draw] Missing shader or texture\n";
        return;
    }

    std::cout << "[SpriteComponent] Draw called" << std::endl;

    shader->Use();
    glActiveTexture(GL_TEXTURE0);
    texture->Bind();

    GLint loc = glGetUniformLocation(shader->GetID(), "tex");
    if (loc < 0) {
        std::cerr << "[Error] Uniform 'tex' not found in shader\n";
    }else{
        glUniform1i(loc, 0);
    }

    //glm::mat4 model = transform.GetTransformMatrix();
    glm::mat4 agentTransform = owner->GetTransform().GetTransformMatrix(); // world transform
    glm::mat4 localTransform = transform.GetTransformMatrix();             // local offset
    //glm::mat4 model = agentTransform * localTransform;
    glm::mat4 model = agentTransform * glm::translate(glm::mat4(1.0f), glm::vec3(transform.position, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(transform.scale, 1.0f));
    glm::mat4 view = glm::mat4(1.0f); // Replace with actual view later

    shader->SetMat4("model", glm::value_ptr(model));
    shader->SetMat4("view", glm::value_ptr(view));
    shader->SetMat4("projection", glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "[SpriteComponent::Draw] OpenGL error: " << err << "\n";
    }
}

void SpriteComponent::DrawImGui() {
    ImGui::Text("Sprite");
    ImGui::Separator();

    ImGui::Text("Name");
    ImGui::SameLine();

    char nameBuffer[64];
    strncpy_s(nameBuffer, GetName(), sizeof(nameBuffer));

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

    if (ImGui::InputText("##spriteName", nameBuffer, sizeof(nameBuffer), flags)) {
        if (nameBuffer[0] == '\0') {
            strncpy_s(nameBuffer, GetName(), sizeof(nameBuffer));
        }
        else {
            this->SetName(nameBuffer);
        }
    }

    ImGui::Text("Texture: %s", std::filesystem::path(texturePath).filename().string().c_str());
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
                            texturePath = fullPath;
                            delete texture;
                            texture = new Texture(fullPath.c_str());
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
            }
            };

        DrawDirectory("assets/sprites");

        ImGui::EndPopup();
    }

    ImGui::Checkbox("Render as Foreground", &isForeground);

    ImGui::Separator();
    ImGui::Text("Transform");
    ImGui::Text("Position: ");
    ImGui::SameLine();
    glm::vec2 pos = transform.position;
    if (ImGui::DragFloat2("##position", glm::value_ptr(pos), 1.0f)) {
        transform.position = pos;
    }

    ImGui::Text("Rotation: ");
    ImGui::SameLine();
    float rot = transform.rotation;
    if (ImGui::DragFloat("##rotation", &rot, 0.1f)) {
        transform.rotation = rot;
    }

    ImGui::Text("Scale:    ");
    ImGui::SameLine();
    glm::vec2 scale = transform.scale;
    if (ImGui::DragFloat2("##scale", glm::value_ptr(scale), 0.1f)) {
        transform.scale = scale;
    }

    if (ImGui::Button("Delete Component")) {
        MarkToDelete();
    }
}

void SpriteComponent::SetVelocity(float x, float y) {
    velocity = { x, y };
}

void SpriteComponent::Update(float deltaTime){
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

    glm::vec2 center = owner->GetTransform().position + transform.position;
    glm::vec2 halfSize = transform.scale * 0.5f;

    glm::vec2 min = center - halfSize;
    glm::vec2 max = center + halfSize;

    bool mouseOver = worldMouse.x >= min.x && worldMouse.x <= max.x &&
        worldMouse.y >= min.y && worldMouse.y <= max.y;

    std::cout << "[SpriteComponent] worldMouse: " << worldMouse.x << ", " << worldMouse.y
        << " | center: " << center.x << ", " << center.y
        << " | transform.pos: " << transform.position.x << ", " << transform.position.y
        << " | selected: " << mouseOver << "\n";

    if (!isMouseDown && interaction.currentAction == SpriteAction::None) {
        interaction.isSelected = mouseOver;
    }

    if (isMouseDown && interaction.currentAction == SpriteAction::None && mouseOver) {
        interaction.currentAction = SpriteAction::Drag;
        interaction.dragOffset = (worldMouse - center) / transform.scale;
    }

    if (!isMouseDown) {
        interaction.currentAction = SpriteAction::None;
    }

    if (interaction.currentAction == SpriteAction::Drag) {
        glm::vec2 delta = (worldMouse - owner->GetTransform().position) - (interaction.dragOffset * transform.scale);
        transform.position = delta;
    }
}

void SpriteComponent::Serialize(std::ofstream& out) const {
    out << "name " << this->GetName() << "\n";
    out << "texture " << texturePath << "\n";
    out << "enabled " << (enabled ? 1 : 0) << "\n";
    out << "transform "
        << transform.position.x << " " << transform.position.y << " "
        << transform.rotation << " "
        << transform.scale.x << " " << transform.scale.y << "\n";
    out << "---\n";
}

void SpriteComponent::Deserialize(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line == "---") break;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "name") {
            std::string tempName;
            std::getline(iss, tempName);
            if (!tempName.empty() && tempName[0] == ' ') tempName.erase(0, 1);
            strncpy_s(name, tempName.c_str(), sizeof(name));
        }
        else if (token == "texture") {
            std::getline(iss, texturePath);
            if (!texturePath.empty() && texturePath[0] == ' ') texturePath.erase(0, 1);
            delete texture;
            texture = new Texture(texturePath.c_str());
        }
        else if (token == "enabled") {
            int state;
            iss >> state;
            enabled = (state != 0);
        }
        else if (token == "transform") {
            iss >> transform.position.x >> transform.position.y;
            iss >> transform.rotation;
            iss >> transform.scale.x >> transform.scale.y;
        }
        else {
            break;
        }
    }
}