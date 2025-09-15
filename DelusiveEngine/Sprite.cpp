#include "Sprite.h"
#include "DelusiveRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb/stb_image.h>
#include <fstream>
#include <sstream>


float quadVertices[] = {
    // pos       // tex
    -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  1.0f, 1.0f,

     0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.0f, 0.0f
};

Sprite::Sprite(const char* texturePath){
    stbi_set_flip_vertically_on_load(true);
    shader = new Shader("shaders\\vertex.glsl", "C:\\Users\\Demon Teddy\\Documents\\Programs\\DelusiveEngine\\DelusiveEngine\\shaders\\fragment.glsl");
    texture = new Texture(texturePath);
    this->texturePath = texturePath;
    colliders.push_back(collider);

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

void Sprite::SetPosition(float x, float y) { position = { x, y }; }
void Sprite::SetScale(float sx, float sy) { scale = { sx, sy }; }
void Sprite::SetRotation(float angle) { rotation = angle; }

void Sprite::Draw(const glm::mat4& projection) const {
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
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(scale, 1.0f));
    glm::mat4 spriteProjection = glm::mat4(1.0f);
    glm::mat4 view = camera.GetViewMatrix();

    shader->SetMat4("model", glm::value_ptr(model));
    shader->SetMat4("view", glm::value_ptr(view));
    shader->SetMat4("projection", glm::value_ptr(spriteProjection));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    for (const Collider& col : colliders) {
        //renderer.Draw(col, projection);
        //renderer.DrawCenterHandle(col.position + col.size * 0.5f, projection);
    }
}

Sprite::~Sprite() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    delete texture;
    delete shader;
    //glDeleteTextures(1, &texture);
}

void Sprite::SetVelocity(float x, float y) {
    velocity = { x, y };
}

void Sprite::UpdateCollider() {
    for (Collider& col : colliders) {
        col.SetPosition(position + col.GetOffset());
        col.SetSize(col.GetSize());
    }
}

const Collider& Sprite::GetCollider() const{
    return collider;
}

//TODO: Support more than one collider per sprite
void Sprite::SetColliderOffset(glm::vec2 offset) {
    collider.SetOffset(offset);
}

glm::vec2 Sprite::GetColliderOffset() {
    return collider.GetOffset();
}

void Sprite::SetColliderSize(glm::vec2 size) {
    collider.SetSize(size);
}

glm::vec2 Sprite::GetColliderSize() {
    return collider.GetSize();
}

bool Sprite::SaveToFile(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "Failed to open file for writing: " << path << std::endl;
        return false;
    }

    out << "texture: " << this->texturePath << "\n";
    out << "colliders: " << colliders.size() << "\n";

    for (Collider col : colliders) {
        out << col.GetOffset().x << " " << col.GetOffset().y << " "
            << col.GetSize().x << " " << col.GetSize().y << "\n";
    }

    return true;
}

bool Sprite::LoadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open file for reading: " << path << std::endl;
        return false;
    }

    std::string line;
    colliders.clear();

    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "texture:") {
            iss >> texturePath;
        }
        else if (keyword == "colliders:") {
            size_t count;
            iss >> count;

            for (size_t i = 0; i < count; ++i) {
                std::getline(in, line);
                std::istringstream colStream(line);
                glm::vec2 offset, size;
                colStream >> offset.x >> offset.y >> size.x >> size.y;

                Collider c;
                c.SetOffset(offset);
                c.SetSize(size);
                colliders.push_back(c);
            }
        }
    }

    return true;
}

std::vector<Collider>& Sprite::GetColliders(){
    return colliders;
}