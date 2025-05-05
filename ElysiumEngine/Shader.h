#pragma once
#include <string>
#include <GL/glew.h>

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    GLuint GetID();
    void Use() const;

    void SetMat4(const std::string& name, const float* value) const;
    void CheckCompileErrors(GLuint, const std::string&);
    void CheckLinkErrors(GLuint program);
private:
    GLuint shaderProgram = 0;
};
