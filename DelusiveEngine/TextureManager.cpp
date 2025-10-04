#include "TextureManager.h"
#include "Texture.h"
#include <cassert>
#include <filesystem>

GLuint TextureManager::Load(const std::string& path) {
    auto it = cache.find(path);
    if (it != cache.end()) {
        //std::cout << "TextureManager: Loaded cached texture: " << path << "\n";
        return it->second->ID;
    }

    auto texture = std::make_unique<Texture>(path.c_str());
    if (texture->ID == 0) {
        std::cerr << "TextureManager::Load ERROR — Texture not loaded: " << path << "\n";
        return 0;
    }

    GLuint texID = texture->ID;
    cache[path] = std::move(texture);
    //std::cout << "TextureManager: Loaded texture: " << path << "\n";
    return texID;
}

void TextureManager::UnloadAll() {
    cache.clear(); // textures deleted by destructor
}

void TextureManager::Bind(const std::string& path, GLuint unit = 0) {
    GLuint id = Load(path);
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
}

GLuint TextureManager::LoadFromFile(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        std::cerr << "TextureManager ERROR: File does not exist: " << path << "\n";
        return 0;
    }

    Texture tex(path.c_str());
    if (tex.ID == 0) {
        std::cerr << "TextureManager ERROR: Failed to load texture: " << path << "\n";
    }

    return tex.ID;
}