#pragma once
#include <GL/glew.h>
#include <iostream>

class Texture {
public:
    GLuint ID;
    Texture(const char* imagePath);
    ~Texture();
    void Bind() const;
};
