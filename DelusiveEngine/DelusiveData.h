#pragma once
#include <string>
#include <GL/glew.h>
#include "Texture.h"
#include "DelusiveMacros.h"
#include "Shader.h"
#include "Font.h"
#include <DelusiveExternal/BehaviourScript.h>
#include "ScriptManager.h"
#include "UUID.h"
#include "UIElement.h"
#include <memory>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

struct RenderCommand {
    glm::mat4 modelMatrix;
    glm::vec4 color;
    GLuint textureID;
    int layer;
    bool isUI;
};

struct DelusiveTexture {
	std::string texturePath = "";
    GLuint textureID = 0;
};

struct DelusiveFont {
    std::string fontPath = "";
    std::string previousFontPath = "";
    float fontSize = 16.0f;            // desired draw size in pixels (runtime scaling)
    float loadedPixelHeight = 0.0f;    // the pixel height used to load glyphs

    GLuint VAO = 0, VBO = 0;
    Shader* shader = nullptr;
    std::unique_ptr<Font> font;        // your Font class (owns glyph textures/metrics)

    DelusiveFont() = default;
    ~DelusiveFont() { Cleanup(); }

    void Cleanup() {
        if (VAO) { glDeleteVertexArrays(1, &VAO); VAO = 0; }
        if (VBO) { glDeleteBuffers(1, &VBO); VBO = 0; }
        if (shader) { delete shader; shader = nullptr; }
        font.reset();
    }

    // Create VAO/VBO and set sampler uniform (call after GL context ready)
    void Init(const std::string& shaderVert = DEFAULT_TEXT_VERT,
        const std::string& shaderFrag = DEFAULT_TEXT_FRAG)
    {
        // cleanup any leftover GL objects
        Cleanup();

        shader = new Shader(shaderVert.c_str(), shaderFrag.c_str());
        shader->Use();

        // Ensure sampler 'tex' uses texture unit 0
        GLint loc = glGetUniformLocation(shader->GetID(), "tex");
        if (loc >= 0) glUniform1i(loc, 0);

        // Create a dynamic VBO sized for a single glyph quad (6 verts * 4 floats)
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

        // layout matches your text.vert expectations:
        // location 0 = vec2 position, location 1 = vec2 uv
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // Load/reload font atlas. Calls Init() if GL objects are missing.
    bool SetFont(const std::string& path, float pixelHeight) {
        fontPath = path;

        if (!shader || VAO == 0 || VBO == 0) {
            Init();
        }

        // Important for glyph uploads
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        auto tmp = std::make_unique<Font>();
        if (!tmp->LoadFromFile(path, pixelHeight)) {
            std::cerr << "[DelusiveFont] Failed to load font: " << path << std::endl;
            return false;
        }

        font = std::move(tmp);
        loadedPixelHeight = pixelHeight;

        // If caller hasn't set a runtime fontSize, default it to loaded height
        if (fontSize <= 0.0f) fontSize = loadedPixelHeight;

        return true;
    }

    // Copy values and re-init GL objects + atlas
    void CloneFrom(const DelusiveFont& other) {
        fontPath = other.fontPath;
        previousFontPath = other.previousFontPath;
        fontSize = other.fontSize;
        loadedPixelHeight = other.loadedPixelHeight;

        Init();
        if (!fontPath.empty()) {
            SetFont(fontPath, loadedPixelHeight > 0.0f ? loadedPixelHeight : fontSize);
        }
    }

    // Draw uses the exact math your original UILabel used (bearing, size, advance)
    // projection should be an orthographic matrix (screen coords).
    void DrawText(const std::string& text,
        const glm::vec2& position,
        const glm::vec4& color,
        const glm::mat4& projection)
    {
        if (!shader || !font) return;

        // enable blending for glyphs (simple management: enable if disabled, then restore)
        GLboolean wasBlend = glIsEnabled(GL_BLEND);
        if (!wasBlend) glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader->Use();
        shader->SetMat4("projection", glm::value_ptr(projection));
        shader->SetVec4("uColor", color);

        // identity model (positions are screen-space)
        glm::mat4 model(1.0f);
        shader->SetMat4("model", glm::value_ptr(model));

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        float x = position.x;
        float y = position.y;
        const float s = (loadedPixelHeight > 0.0f) ? (fontSize / loadedPixelHeight) : 1.0f;

        for (unsigned char uc : text) {
            const Character& ch = font->GetCharacter(uc);
            // skip missing glyphs (Font should return empty Character)
            if (ch.textureID == 0 && ch.size.x == 0 && ch.size.y == 0) continue;

            float xpos = x + ch.bearing.x * s;
            float ypos = y - (ch.size.y - ch.bearing.y) * s;
            float w = ch.size.x * s;
            float h = ch.size.y * s;

            // match your previous vertex order/uvs
            float verts[6][4] = {
                { xpos,     ypos,     0.0f, 1.0f }, // bottom-left
                { xpos,     ypos + h, 0.0f, 0.0f }, // top-left
                { xpos + w, ypos + h, 1.0f, 0.0f }, // top-right

                { xpos,     ypos,     0.0f, 1.0f }, // bottom-left
                { xpos + w, ypos + h, 1.0f, 0.0f }, // top-right
                { xpos + w, ypos,     1.0f, 1.0f }  // bottom-right
            };

            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // advance is in pixels for the loaded font
            x += ch.advance * s;
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        if (!wasBlend) glDisable(GL_BLEND);
    }
};

struct DelusiveScript {
	std::string scriptName;
    std::unique_ptr<BehaviourScript> script;
	ScriptManager* manager = nullptr;
};

struct DelusiveUIElement {
    std::string name;
    UUID* id;
    UIElement* link = nullptr;
};

struct DelusiveUIPrototype {
    std::string type;
    std::unique_ptr<UIElement> element;
};