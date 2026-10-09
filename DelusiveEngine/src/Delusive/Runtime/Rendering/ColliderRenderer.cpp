#include <Delusive/Internal/Rendering/ColliderRenderer.h>
#include <Delusive/Internal/Rendering/Shader.h>
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

    glGenVertexArrays(1, &pointsVAO);
    glGenBuffers(1, &pointsVBO);
    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
}

ColliderRenderer::~ColliderRenderer() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &pointsVBO);
    glDeleteVertexArrays(1, &pointsVAO);
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
        DrawHandle(PhysicsSystem::BuildShape(collider).center, projection);
    }

    for (const auto& handle : collider.GetHandles()) DrawHandle(handle.position, projection);
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

    DrawPoints(points.data(), points.size(), GL_LINE_STRIP, projection);
}

void ColliderRenderer::DrawLine(const ColliderComponent& collider, const glm::mat4& projection) const {
    const WorldShape line = PhysicsSystem::BuildShape(collider);
    glm::vec2 points[2] = { line.center, line.end };

    DrawPoints(points, 2, GL_LINES, projection);
}

void ColliderRenderer::DrawPoints(const glm::vec2* points, std::size_t count, GLenum mode, const glm::mat4& projection) const {
    shader->Use();
    shader->SetMat4("model", glm::value_ptr(glm::mat4(1.0f)));
    shader->SetMat4("projection", glm::value_ptr(projection));
    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec2), points, GL_DYNAMIC_DRAW);
    glDrawArrays(mode, 0, static_cast<GLsizei>(count));
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