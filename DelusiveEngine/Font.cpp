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

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; ++c) {
        int w, h, xoff, yoff;
        unsigned char* bmp = stbtt_GetCodepointBitmap(&fontInfo, 0, scale, c, &w, &h, &xoff, &yoff);

        GLuint texID;
        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, bmp);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int advWidth, lsb;
        stbtt_GetCodepointHMetrics(&fontInfo, c, &advWidth, &lsb);

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&fontInfo, c, scale, scale, &x0, &y0, &x1, &y1);

        stbtt_FreeBitmap(bmp, nullptr);

        Character ch{
            texID,
            glm::ivec2(x1 - x0, y1 - y0),   // size
            glm::ivec2(x0, -y0),             // bearing = (x0,y0) RELATIVE TO BASELINE
            advWidth * scale                 // float
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