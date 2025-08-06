#pragma once
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    GLuint GetID();
    void Use() const;

    void SetMat4(const std::string&, const float*) const;
    void SetMat4(const std::string&, const glm::mat4&) const;
    void SetVec4(const std::string&, const glm::vec4&) const;
    void SetVec2(const std::string&, const glm::vec2&) const;
    void SetInt(const std::string&, int) const;

    void CheckCompileErrors(GLuint, const std::string&);
    void CheckLinkErrors(GLuint);
private:
    GLuint shaderProgram = 0;
};
