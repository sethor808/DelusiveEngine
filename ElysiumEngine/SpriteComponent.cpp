#include "SpriteComponent.h"
#include "Agent.h"
#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

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
    SetScale(64.0f, 64.0f);

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
    glm::mat4 model = agentTransform * localTransform;
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

    // Copy current name into buffer
    strncpy_s(nameBuffer, GetName(), sizeof(nameBuffer));

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

    if (ImGui::InputText("##spriteName", nameBuffer, sizeof(nameBuffer), flags)) {
        // This triggers only when Enter is pressed
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
        for (const auto& entry : std::filesystem::directory_iterator("assets/sprites")) {
            if (entry.is_regular_file()) {
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
        ImGui::EndPopup();
    }

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
}

void SpriteComponent::SetVelocity(float x, float y) {
    velocity = { x, y };
}

void SpriteComponent::Update(float deltaTime){
    transform.position += velocity * deltaTime;
    //Probably move camera here
}