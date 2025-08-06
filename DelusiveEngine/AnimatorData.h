#pragma once
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>

struct FlagChange {
    std::string flag;
    bool set = false;
    float duration = 0.0f;
};

struct ComponentMod {
    uint64_t componentID = 0;
    bool enabled = true;
    glm::vec2 positionOffset = { 0, 0 };
    glm::vec2 scale = { 1, 1 };
    float rotation = 0.0f;
    std::string texturePath = "";
};

struct AnimationFrame {
    float duration = 0.1f;
    bool inputLock = false;
    std::vector<FlagChange> flagChanges;
    std::vector<ComponentMod> componentOverrides;

    //Texture preview control
    GLuint previewTexture = 0;
    bool dirty = true;
};

struct EntryCondition {
    std::string flag;
    bool expectedValue;
};

struct AnimationBranch {
    std::string name = "Unnamed";
    bool loop = false;
    bool lockInput = false;
    std::vector<EntryCondition> conditions;
    std::vector<AnimationFrame> frames;
};

struct AnimatorData {
    std::string name = "UnnamedAnimation";
    std::string defaultAgentPath;

    std::vector<std::string> flags;
    std::vector<AnimationBranch> branches;
};