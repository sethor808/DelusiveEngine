#include "ColliderRenderer.h"
#include <gl/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

ColliderRenderer::ColliderRenderer() {
    // Static 1x1 square for reuse
    float quad[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
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

void ColliderRenderer::Draw(const Collider& collider, const glm::mat4& projection) const{
    shader->Use();

    GLuint programID = shader->GetID();
    std::cout << "[Shader] Using program ID: " << programID << "\n";

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(collider.position, 0.0f));
    model = glm::scale(model, glm::vec3(collider.size, 1.0f));

    GLint modelLoc = glGetUniformLocation(programID, "model");
    GLint projLoc = glGetUniformLocation(programID, "projection");
    GLint colorLoc = glGetUniformLocation(programID, "color");

    std::cout << "[Uniforms] model: " << modelLoc
        << " | projection: " << projLoc
        << " | color: " << colorLoc << std::endl;

    if (modelLoc >= 0)
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    if (projLoc >= 0)
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    if (colorLoc >= 0)
        glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f); // RED

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL Error after draw: " << err << std::endl;
    }
}

void ColliderRenderer::DrawCenterHandle(const glm::vec2& center, const glm::mat4& projection) const {
    shader->Use();

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
    model = glm::scale(model, glm::vec3(6.0f, 6.0f, 1.0f));

    shader->SetMat4("model", glm::value_ptr(model));
    shader->SetMat4("projection", glm::value_ptr(projection));

    GLint colorLoc = glGetUniformLocation(shader->GetID(), "color");
    if (colorLoc != -1) {
        glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, 1.0f);
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}