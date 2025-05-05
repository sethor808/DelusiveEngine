#pragma once
#include "Shader.h"
#include "Texture.h"
#include "ColliderRenderer.h"
#include "Camera.h"
#include "Collider.h"
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>

class Sprite {
public:
    Sprite(const char* texturePath);
    ~Sprite();
    void SetPosition(float x, float y);
    void SetScale(float sx, float sy);
    void SetRotation(float angle);
    void Draw(const ColliderRenderer& renderer, const glm::mat4& projection) const;
    void SetVelocity(float x, float y);
    void Update(float dt) {
        position += velocity * dt;
        camera.Move(position.x, position.y);
    }
    void UpdateCollider();
    const Collider& GetCollider() const;
    std::vector<Collider>& GetColliders();
    glm::vec2 GetColliderOffset();
    void SetColliderOffset(glm::vec2);
    glm::vec2 GetColliderSize();
    void SetColliderSize(glm::vec2);
    bool SaveToFile(const std::string& path) const;
    bool LoadFromFile(const std::string& path);
private:
    std::vector<Collider> colliders;
    std::string texturePath;
    GLuint VAO, VBO;
    Shader* shader;
    Texture* texture;
    glm::vec2 position, scale;
    glm::vec2 velocity = { 0.0f, 0.0f };
    float rotation;
    Camera2D camera;
    Collider collider;
};