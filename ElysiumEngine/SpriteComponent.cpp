#include "SpriteComponent.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>

float quadVertices[] = {
    // pos       // tex
    -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  1.0f, 1.0f,

     0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.0f, 0.0f
};

SpriteComponent::SpriteComponent(const char* texturePath){
        stbi_set_flip_vertically_on_load(true);
        shader = new Shader("shaders\\vertex.glsl", "C:\\Users\\Demon Teddy\\Documents\\Programs\\ElysiumEngine\\ElysiumEngine\\shaders\\fragment.glsl");
        texture = new Texture(texturePath);
        this->texturePath = texturePath;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
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

void SpriteComponent::Draw(const glm::mat4& projection) const {
    shader->Use();
    texture->Bind();
    glUniform1i(glGetUniformLocation(shader->GetID(), "tex"), 0);

    // Log (for one-time debug)
    static bool printed = false;
    if (!printed) {
        std::cout << "[Draw] Sprite draw started\n";
        printed = true;
    }

    // Build model matrix: translate, rotate, scale
    glm::mat4 modelMatrix = transform.GetTransformMatrix();
    glm::mat4 view = glm::mat4(1.0f); //Eventually pass the camera in

    shader->SetMat4("model", glm::value_ptr(modelMatrix));
    shader->SetMat4("view", glm::value_ptr(view));
    shader->SetMat4("projection", glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void SpriteComponent::SetVelocity(float x, float y) {
    velocity = { x, y };
}

void SpriteComponent::Update(float deltaTime){
    transform.position += velocity * deltaTime;
    //Probably move camera here
}