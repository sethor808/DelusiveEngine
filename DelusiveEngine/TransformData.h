#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct TransformComponent {
    glm::vec2 position{ 0.0f, 0.0f };
    float rotation{ 0.0f }; // radians
    glm::vec2 scale{ 1.0f, 1.0f };

    glm::mat4 GetTransformMatrix() const {
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(scale, 1.0f));
        return model;
    }
};