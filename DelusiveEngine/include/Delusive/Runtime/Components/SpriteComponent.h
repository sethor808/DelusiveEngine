#pragma once
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/Editor/EditorInterface.h>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>

class SpriteComponent : public Component {
public:
    bool isForeground = false;

    SpriteComponent(DelusiveInstance&);
    SpriteComponent() = delete;

    SpriteComponent(const SpriteComponent&) = delete;
    SpriteComponent& operator=(const SpriteComponent&) = delete;
    SpriteComponent(SpriteComponent&&) noexcept = default;
    SpriteComponent& operator=(SpriteComponent&&) noexcept = default;

    void Init();

    void RegisterProperties() override;

    void SetTexturePath(const std::string&);
    //Loading only sets the path - this turns it into a texture
    void Deserialize(DelusiveParser::DataBlock& in) override;
    void SetPosition(float x, float y);
    void SetScale(float sx, float sy);
    void SetRotation(float angle);
    void Draw(const glm::mat4& projection) const override;
    void DrawImGui() override;
    void SetVelocity(float x, float y);
    void Update(float) override;
    void SetLocalTransform(const glm::vec2&, const glm::vec2&, float) override;
    void HandleMouse(const glm::vec2&, bool) override;

    const char* GetType() const override {
        return "SpriteComponent";
    }
private:
    InteractionState interaction;
	DelusiveTexture textureData;
    
    int renderOrder = 0;
    glm::vec2 velocity = { 0.0f, 0.0f };
};
