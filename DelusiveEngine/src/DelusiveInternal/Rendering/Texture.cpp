#include <Delusive/Internal/Rendering/Texture.h>
#include <GL/glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <filesystem>

Texture::Texture(const char* path) {
    if (!path || path == "") {
        std::cout << "Path is empty!" << std::endl;
        return;
    }

    //stbi_set_flip_vertically_on_load(true);
    //path = "C:/Users/Demon Teddy/Documents/Programs/DelusiveEngine/DelusiveEngine/assets/sprites/star.jpg";
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
        std::cout << "CWD: " << std::filesystem::current_path() << std::endl;
        std::cout << "Path: " << path << std::endl;
        std::cerr << "Failed to load texture: " << path << std::endl;
        std::cerr << "stb_image reason: " << stbi_failure_reason() << std::endl;
    }

    stbi_image_free(data);

    GLint currentContext;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentContext);
    std::cout << "Current texture binding: " << currentContext << "\n";

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error during texture load: " << err << "\n";
    }
}

Texture::~Texture() {
    glDeleteTextures(1, &ID);
}

void Texture::Bind() const {
    glBindTexture(GL_TEXTURE_2D, ID);
}