#include "ColliderRenderer.h"
#include "Agent.h"
#include <gl/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

ColliderRenderer::ColliderRenderer() {
    handleSize = handleSize / Renderer::GetPixelScale();

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

    GLint colorLoc = glGetUniformLocation(shader->GetID(), "color");
    switch (collider.GetColliderType()) {
    case ColliderType::Solid:
        glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f); // Red
        break;
    case ColliderType::Hitbox:
        glUniform4f(colorLoc, 1.0f, 0.0f, 1.0f, 1.0f); // Magenta
        break;
    case ColliderType::Hurtbox:
        glUniform4f(colorLoc, 0.0f, 0.5f, 1.0f, 1.0f); // Blue-ish
        break;
    }

    ShapeType shape = collider.GetShapeType();
    switch (shape) {
    case ShapeType::Box:
        DrawBox(collider, projection);
        break;
    case ShapeType::Circle:
        DrawCircle(collider, projection);
        break;
    case ShapeType::Line:
        DrawLine(collider, projection);
        break;
    }

    if (collider.CheckCenterRender()) {
        glm::vec4 worldCenter = collider.GetOwner()->GetTransform().GetTransformMatrix() * glm::vec4(collider.transform.position, 0.0f, 1.0f);
        DrawCenterHandle(glm::vec2(worldCenter), projection);
    }

    DrawHandles(collider, projection);
}

void ColliderRenderer::DrawBox(const ColliderComponent& collider, const glm::mat4& projection) const {
    glm::mat4 model = collider.GetOwner()->GetTransform().GetTransformMatrix() * collider.transform.GetTransformMatrix();
    shader->SetMat4("model", glm::value_ptr(model));
    shader->SetMat4("projection", glm::value_ptr(projection));
    glUniform4f(glGetUniformLocation(shader->GetID(), "color"), 1.0f, 0.0f, 0.0f, 1.0f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void ColliderRenderer::DrawCircle(const ColliderComponent& collider, const glm::mat4& projection) const {
    glm::vec2 center = collider.transform.position;
    float radius = collider.transform.scale.x * 0.5f;
    glm::mat4 agentMatrix = collider.GetOwner()->GetTransform().GetTransformMatrix();

    const int segments = 32;
    std::vector<glm::vec2> points;
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * glm::two_pi<float>();
        glm::vec2 local = center + glm::vec2(cos(angle), sin(angle)) * radius;
        glm::vec4 world = agentMatrix * glm::vec4(local, 0.0f, 1.0f);
        points.push_back(glm::vec2(world));
    }

    shader->Use();
    shader->SetMat4("model", glm::value_ptr(glm::mat4(1.0f)));
    shader->SetMat4("projection", glm::value_ptr(projection));
    glUniform4f(glGetUniformLocation(shader->GetID(), "color"), 0.0f, 1.0f, 0.0f, 1.0f); // Green

    GLuint circleVBO, circleVAO;
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);

    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec2), points.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)points.size());

    glDeleteBuffers(1, &circleVBO);
    glDeleteVertexArrays(1, &circleVAO);
}

void ColliderRenderer::DrawLine(const ColliderComponent& collider, const glm::mat4& projection) const {
    glm::vec2 start = collider.transform.position;
    glm::vec2 dir = glm::vec2(cos(collider.transform.rotation), sin(collider.transform.rotation));
    float length = collider.transform.scale.x;
    glm::vec2 end = start + dir * length;

    glm::mat4 agentMatrix = collider.GetOwner()->GetTransform().GetTransformMatrix();
    glm::vec2 worldStart = glm::vec2(agentMatrix * glm::vec4(start, 0.0f, 1.0f));
    glm::vec2 worldEnd = glm::vec2(agentMatrix * glm::vec4(end, 0.0f, 1.0f));

    glm::vec2 points[2] = { worldStart, worldEnd };

    shader->Use();
    shader->SetMat4("model", glm::value_ptr(glm::mat4(1.0f)));
    shader->SetMat4("projection", glm::value_ptr(projection));
    glUniform4f(glGetUniformLocation(shader->GetID(), "color"), 1.0f, 1.0f, 0.0f, 1.0f); // Yellow

    GLuint lineVBO, lineVAO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_LINES, 0, 2);

    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &lineVAO);
}

void ColliderRenderer::DrawCenterHandle(const glm::vec2& center, const glm::mat4& projection) const {
    shader->Use();
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(handleSize, handleSize, 1.0f));

    // Try a bigger handle or consider screen-space scaling
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
    switch (collider.GetShapeType()) {
    case ShapeType::Box:
        DrawBoxHandles(collider, projection);
        break;
    case ShapeType::Circle:
        DrawCircleHandles(collider, projection);
        break;
    case ShapeType::Line:
        DrawLineHandles(collider, projection);
        break;
    }
}

void ColliderRenderer::DrawBoxHandles(const ColliderComponent& collider, const glm::mat4& projection) const {
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

void ColliderRenderer::DrawCircleHandles(const ColliderComponent& collider, const glm::mat4& projection) const {
    glm::vec2 center = collider.transform.position;
    float radius = collider.transform.scale.x * 0.5f;

    glm::mat4 agentMatrix = collider.GetOwner()->GetTransform().GetTransformMatrix();

    // Center handle
    glm::vec4 worldCenter = agentMatrix * glm::vec4(center, 0.0f, 1.0f);
    DrawHandle(glm::vec2(worldCenter), projection);

    // Edge handle (right side)
    glm::vec2 handlePosLocal = center + glm::vec2(radius, 0.0f);
    glm::vec4 worldHandle = agentMatrix * glm::vec4(handlePosLocal, 0.0f, 1.0f);
    DrawHandle(glm::vec2(worldHandle), projection);;
}

void ColliderRenderer::DrawLineHandles(const ColliderComponent& collider, const glm::mat4& projection) const {
    glm::vec2 start = collider.transform.position;
    glm::vec2 dir = glm::vec2(cos(collider.transform.rotation), sin(collider.transform.rotation));
    float length = collider.transform.scale.x;
    glm::vec2 end = start + dir * length;

    glm::mat4 agentMatrix = collider.GetOwner()->GetTransform().GetTransformMatrix();
    glm::vec2 worldStart = glm::vec2(agentMatrix * glm::vec4(start, 0.0f, 1.0f));
    glm::vec2 worldEnd = glm::vec2(agentMatrix * glm::vec4(end, 0.0f, 1.0f));
    glm::vec2 worldCenter = (worldStart + worldEnd) * 0.5f;

    // Start handle
    DrawHandle(worldStart, projection);
    // End handle
    DrawHandle(worldEnd, projection);
    // Center reposition handle
    DrawHandle(worldCenter, projection);
}

void ColliderRenderer::DrawHandle(const glm::vec2& center, const glm::mat4& projection) const {
    shader->Use();
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(handleSize, handleSize, 1.0f));

    shader->SetMat4("model", glm::value_ptr(model));
    shader->SetMat4("projection", glm::value_ptr(projection));

    GLint colorLoc = glGetUniformLocation(shader->GetID(), "color");
    if (colorLoc != -1)
        glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}