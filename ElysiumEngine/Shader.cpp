#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::cout << "[Shader] Loading: " << vertexPath << " and " << fragmentPath << std::endl;

    std::string vertexCode, fragmentCode;
    std::ifstream vFile(vertexPath), fFile(fragmentPath);

    if (!vFile || !fFile) {
        std::cerr << "Failed to open shader files.\n";
        return;
    }

    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();
    vertexCode = vStream.str();
    fragmentCode = fStream.str();

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // Vertex shader
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    CheckCompileErrors(vertex, "VERTEX");

    // Fragment shader
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    CheckCompileErrors(fragment, "FRAGMENT");

    // Shader program
    this->shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
    CheckLinkErrors(shaderProgram);

    std::cout << "[Shader] Created program ID: " << shaderProgram << std::endl;

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    glDeleteProgram(shaderProgram);
}

GLuint Shader::GetID() {
    return shaderProgram;
}

void Shader::Use() const {
    glUseProgram(shaderProgram);
    std::cout << "[Shader] Using program ID: " << shaderProgram << "\n";
}

void Shader::SetMat4(const std::string& name, const float* value) const {
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, name.c_str()), 1, GL_FALSE, value);
}

// Helper error checking
void Shader::CheckCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "Shader Compilation Error(" << type << ") :\n" << log << "\n";
    }
}

void Shader::CheckLinkErrors(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(program, 1024, nullptr, log);
        std::cerr << "Shader Linking Error : \n" << log << "\n";
    }
}