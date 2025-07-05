#include "ColliderRenderer.h"
#include "Agent.h"
#include <gl/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

ColliderRenderer::ColliderRenderer() {
    // Static 1x1 square for reuse
    float quad[] = {
    -0.5f, -0.5f,
     0.5f, -0.5f,
     0.5f,  0.5f,
    -0.5f,  0.5f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    shader = new Shader("shaders\\collider_vert.glsl", "shaders\\collider_frag.glsl");
    std::cout << "[ColliderRenderer] Shader program ID: " << shader->GetID() << std::endl;

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

ColliderRenderer::~ColliderRenderer() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    delete shader;
}

void ColliderRenderer::Draw(const ColliderComponent& collider, const glm::mat4& projection) const{
    shader->Use();

    // Combine agent's world transform with collider's local transform
    glm::mat4 agentMatrix = collider.GetOwner()->GetTransform().GetTransformMatrix(); // World
    glm::mat4 localMatrix = collider.transform.GetTransformMatrix(); // Local
    glm::mat4 model = agentMatrix * localMatrix; // Combine transforms

    shader->SetMat4("model", glm::value_ptr(model));
    shader->SetMat4("projection", glm::value_ptr(projection));

    GLint colorLoc = glGetUniformLocation(shader->GetID(), "color");
    if (colorLoc != -1)
        glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f); // Red

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    if (collider.CheckCenterRender()) {
        // Transform collider center to world position
        std::cout << "[ColliderRenderer] collider transform: " << collider.transform.position.x << "," << collider.transform.position.y << std::endl;
        glm::vec2 localCenter = collider.transform.position;
        glm::vec4 worldCenter = agentMatrix * glm::vec4(localCenter, 0.0f, 1.0f);
        DrawCenterHandle(glm::vec2(worldCenter), projection);
    }

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "[OpenGL] Error after collider draw: " << err << std::endl;
    }

    DrawHandles(collider, projection);
}

void ColliderRenderer::DrawCenterHandle(const glm::vec2& center, const glm::mat4& projection) const {
    shader->Use();
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(6.0f, 6.0f, 1.0f));

    // Try a bigger handle or consider screen-space scaling
    float handleSize = 12.0f; // or calculate dynamically based on camera zoom
    model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(handleSize, handleSize, 1.0f));

    shader->SetMat4("model", glm::value_ptr(model));
    shader->SetMat4("projection", glm::value_ptr(projection));

    GLint colorLoc = glGetUniformLocation(shader->GetID(), "color");
    if (colorLoc != -1) {
        glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void ColliderRenderer::DrawHandles(const ColliderComponent& collider, const glm::mat4& projection) const {
    const glm::mat4 agentMatrix = collider.GetOwner()->GetTransform().GetTransformMatrix();
    const glm::mat4 localMatrix = collider.transform.GetTransformMatrix();
    const glm::mat4 model = agentMatrix * localMatrix;

    const glm::vec2 size = collider.transform.scale;
    const glm::vec2 center = collider.transform.position;

    // Offset positions (local space, will be transformed)
    std::vector<glm::vec2> handlePoints = {
        center, // Center
        center + glm::vec2(-size.x / 2, 0), // Left
        center + glm::vec2(size.x / 2, 0),  // Right
        center + glm::vec2(0, -size.y / 2), // Bottom
        center + glm::vec2(0, size.y / 2),  // Top
        center + glm::vec2(-size.x / 2, -size.y / 2), // Bottom Left
        center + glm::vec2(size.x / 2, -size.y / 2),  // Bottom Right
        center + glm::vec2(-size.x / 2, size.y / 2),  // Top Left
        center + glm::vec2(size.x / 2, size.y / 2)    // Top Right
    };

    for (const auto& pt : handlePoints) {
        glm::vec4 worldPos = agentMatrix * glm::vec4(pt, 0.0f, 1.0f);
        DrawHandle(glm::vec2(worldPos), projection);
    }
}

void ColliderRenderer::DrawHandle(const glm::vec2& center, const glm::mat4& projection) const {
    shader->Use();
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(6.0f, 6.0f, 1.0f));

    shader->SetMat4("model", glm::value_ptr(model));
    shader->SetMat4("projection", glm::value_ptr(projection));

    GLint colorLoc = glGetUniformLocation(shader->GetID(), "color");
    if (colorLoc != -1)
        glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}