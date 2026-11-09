#include <Delusive/Internal/Rendering/Font.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

Font::~Font() {
    if (atlasID) glDeleteTextures(1, &atlasID);
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

    //Glyphs are packed left to right in rows, one pixel apart
    struct Glyph {
        unsigned char* bitmap;
        int w, h;
        glm::ivec2 slot;
    };
    const int ATLAS_WIDTH = 1024;
    std::vector<Glyph> glyphs(128);
    glm::ivec2 cursor(0);
    int rowHeight = 0;
    for (int c = 0; c < 128; ++c) {
        Glyph& glyph = glyphs[c];
        int xoff, yoff;
        glyph.bitmap = stbtt_GetCodepointBitmap(&fontInfo, 0, scale, c, &glyph.w, &glyph.h, &xoff, &yoff);
        if (cursor.x + glyph.w > ATLAS_WIDTH) {
            cursor = { 0, cursor.y + rowHeight + 1 };
            rowHeight = 0;
        }
        glyph.slot = cursor;
        cursor.x += glyph.w + 1;
        rowHeight = std::max(rowHeight, glyph.h);
    }
    const int atlasHeight = std::max(cursor.y + rowHeight, 1);

    std::vector<unsigned char> pixels(ATLAS_WIDTH * atlasHeight, 0);
    for (int c = 0; c < 128; ++c) {
        const Glyph& glyph = glyphs[c];
        for (int row = 0; row < glyph.h; ++row) {
            std::copy_n(glyph.bitmap + row * glyph.w, glyph.w, pixels.begin() + (glyph.slot.y + row) * ATLAS_WIDTH + glyph.slot.x);
        }
        stbtt_FreeBitmap(glyph.bitmap, nullptr);

        int advWidth, lsb;
        stbtt_GetCodepointHMetrics(&fontInfo, c, &advWidth, &lsb);

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&fontInfo, c, scale, scale, &x0, &y0, &x1, &y1);

        const glm::vec2 atlasSize(ATLAS_WIDTH, atlasHeight);
        Character ch{
            glm::vec2(glyph.slot) / atlasSize,
            glm::vec2(glyph.slot + glm::ivec2(glyph.w, glyph.h)) / atlasSize,
            glm::ivec2(x1 - x0, y1 - y0),   // size
            glm::ivec2(x0, -y0),             // bearing = (x0,y0) RELATIVE TO BASELINE
            static_cast<GLuint>(advWidth * scale)
        };
        characters[static_cast<char>(c)] = ch;
    }

    if (atlasID) glDeleteTextures(1, &atlasID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &atlasID);
    glBindTexture(GL_TEXTURE_2D, atlasID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ATLAS_WIDTH, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

void Font::AppendQuads(const std::string& text, const glm::vec2& position, float scale, std::vector<glm::vec4>& out) const {
    float x = position.x;
    for (char c : text) {
        const Character& ch = GetCharacter(c);
        if (ch.size.x > 0 && ch.size.y > 0) {
            const float left = x + ch.bearing.x * scale;
            const float bottom = position.y - (ch.size.y - ch.bearing.y) * scale;
            const float right = left + ch.size.x * scale;
            const float top = bottom + ch.size.y * scale;

            const glm::vec4 bottomLeft(left, bottom, ch.uvMin.x, ch.uvMax.y);
            const glm::vec4 topLeft(left, top, ch.uvMin.x, ch.uvMin.y);
            const glm::vec4 topRight(right, top, ch.uvMax.x, ch.uvMin.y);
            const glm::vec4 bottomRight(right, bottom, ch.uvMax.x, ch.uvMax.y);
            out.insert(out.end(), { bottomLeft, topLeft, topRight, bottomLeft, topRight, bottomRight });
        }
        x += ch.advance * scale;
    }
}