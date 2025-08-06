#include "Font.h"
#include <vector>
#include <fstream>
#include <iostream>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

Font::~Font() {
    // Clean up textures
    for (auto& [c, ch] : characters) {
        glDeleteTextures(1, &ch.textureID);
    }
}

bool Font::LoadFromFile(const std::string& path, float pixelHeight) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open font file: " << path << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    ttfBuffer.resize(size);
    if (!file.read(reinterpret_cast<char*>(ttfBuffer.data()), size)) {
        std::cerr << "Failed to read font data.\n";
        return false;
    }

    if (!stbtt_InitFont(&fontInfo, ttfBuffer.data(), 0)) {
        std::cerr << "Failed to initialize font.\n";
        return false;
    }

    scale = stbtt_ScaleForPixelHeight(&fontInfo, pixelHeight);
    int ascentRaw, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascentRaw, &descent, &lineGap);
    ascent = ascentRaw * scale;

    for (unsigned char c = 32; c < 127; ++c) {
        int width, height, xoff, yoff;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(&fontInfo, 0, scale, c, &width, &height, &xoff, &yoff);

        GLuint texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&fontInfo, c, &advance, &lsb);
        stbtt_FreeBitmap(bitmap, nullptr);

        Character ch = {
            texID,
            glm::ivec2(width, height),
            glm::ivec2(xoff, yoff),
            static_cast<GLuint>(advance * scale)
        };

        characters[c] = ch;
    }

    return true;
}
const Character& Font::GetCharacter(char c) const {
    auto it = characters.find(c);
    if (it != characters.end()) {
        return it->second;
    }

    static Character defaultChar = {};
    return defaultChar;
}