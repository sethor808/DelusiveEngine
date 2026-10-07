#include <Delusive/Internal/Rendering/ColliderRenderer.h>
#include <Delusive/Runtime/Core/PhysicsSystem.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <iostream>

ColliderRenderer::ColliderRenderer() {
    handleSize = ColliderComponent::HandleSize;

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

    shader = new Shader(DEFAULT_COLL_VERT, DEFAULT_COLL_FRAG);
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
    if(!shader) {
        std::cerr << "[ColliderRenderer] shader is nullptr!\n";
        return;
    }

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
    case ColliderType::Trigger:
        glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f); // Yellow
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
        glm::vec4 worldCenter = collider.GetOwner()->GetTransform().ToMatrix() * glm::vec4(collider.transform->position, 0.0f, 1.0f);
        DrawCenterHandle(glm::vec2(worldCenter), projection);
    }

    DrawHandles(collider, projection);
}

//Shapes come from PhysicsSystem::BuildShape so what is drawn is exactly what collides.
//The color was set per collider type in Draw.
void ColliderRenderer::DrawBox(const ColliderComponent& collider, const glm::mat4& projection) const {
    const WorldShape box = PhysicsSystem::BuildShape(collider);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(box.center, 0.0f));
    model = glm::scale(model, glm::vec3(box.max - box.min, 1.0f));
    shader->SetMat4("model", glm::value_ptr(model));
    shader->SetMat4("projection", glm::value_ptr(projection));
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void ColliderRenderer::DrawCircle(const ColliderComponent& collider, const glm::mat4& projection) const {
    const WorldShape circle = PhysicsSystem::BuildShape(collider);

    const int segments = 32;
    std::vector<glm::vec2> points;
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * glm::two_pi<float>();
        points.push_back(circle.center + glm::vec2(cos(angle), sin(angle)) * circle.radius);
    }

    shader->Use();
    shader->SetMat4("model", glm::value_ptr(glm::mat4(1.0f)));
    shader->SetMat4("projection", glm::value_ptr(projection));

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
    const WorldShape line = PhysicsSystem::BuildShape(collider);
    glm::vec2 points[2] = { line.center, line.end };

    shader->Use();
    shader->SetMat4("model", glm::value_ptr(glm::mat4(1.0f)));
    shader->SetMat4("projection", glm::value_ptr(projection));

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

//Handle positions come from the collider, so what is drawn is exactly what can be grabbed
void ColliderRenderer::DrawBoxHandles(const ColliderComponent& collider, const glm::mat4& projection) const {
    for (const auto& handle : collider.GetHandles()) DrawHandle(handle.position, projection);
}

void ColliderRenderer::DrawCircleHandles(const ColliderComponent& collider, const glm::mat4& projection) const {
    for (const auto& handle : collider.GetHandles()) DrawHandle(handle.position, projection);
}

void ColliderRenderer::DrawLineHandles(const ColliderComponent& collider, const glm::mat4& projection) const {
    for (const auto& handle : collider.GetHandles()) DrawHandle(handle.position, projection);
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