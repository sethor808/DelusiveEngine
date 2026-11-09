#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <stb/stb_truetype.h>

struct Character {
    glm::vec2 uvMin;     // Glyph rect in the font atlas, top-left
    glm::vec2 uvMax;     // Bottom-right
    glm::ivec2 size;     // Width, Height
    glm::ivec2 bearing;  // Offset from baseline
    GLuint advance;      // Advance to next glyph
};

class Font {
public:
    Font() = default;
    ~Font();

    bool LoadFromFile(const std::string& path, float pixelHeight);
    const Character& GetCharacter(char c) const;
    float GetAscent() const { return ascent; }
    //Every glyph lives in this one texture, so a whole string draws at once
    GLuint GetAtlas() const { return atlasID; }
    //Two triangles (x, y, u, v per corner) per glyph, starting on the baseline at position
    void AppendQuads(const std::string& text, const glm::vec2& position, float scale, std::vector<glm::vec4>& out) const;

private:
    std::unordered_map<char, Character> characters;
    GLuint atlasID = 0;
    stbtt_fontinfo fontInfo;

    float ascent = 0.0f;
    float scale = 1.0f;
    std::vector<unsigned char> ttfBuffer;
};