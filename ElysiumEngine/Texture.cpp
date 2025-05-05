#include "Texture.h"
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Texture::Texture(const char* path) {
    stbi_set_flip_vertically_on_load(true);

    int w, h, channels;
    unsigned char* data = stbi_load(path, &w, &h, &channels, 4);
    if (data) {
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cerr << "Failed to load texture: " << path << std::endl;
    }

    stbi_image_free(data);
}

Texture::~Texture() {
    glDeleteTextures(1, &ID);
}

void Texture::Bind() const {
    glBindTexture(GL_TEXTURE_2D, ID);
}