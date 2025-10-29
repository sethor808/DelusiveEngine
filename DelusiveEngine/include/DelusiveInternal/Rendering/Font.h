#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <stb/stb_truetype.h>

struct Character {
    GLuint textureID;
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

private:
    std::unordered_map<char, Character> characters;
    stbtt_fontinfo fontInfo;

    float ascent = 0.0f;
    float scale = 1.0f;
    std::vector<unsigned char> ttfBuffer;
};