#pragma once
#include <GL/glew.h>
#include <iostream>

class Texture {
public:
    GLuint ID = 0;
    //Pixel size of the source image, 0 if it failed to load
    int width = 0;
    int height = 0;
    Texture(const char* imagePath);
    ~Texture();
    void Bind() const;
};
