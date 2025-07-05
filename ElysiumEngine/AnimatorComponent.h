#pragma once
#include "Component.h"
#include "AnimatorData.h" // assumes this holds branches/frames/flags/etc.
#include "Animation.h"

class AnimatorComponent : public Component {
public:
    AnimatorData data;

    AnimatorComponent();
    explicit AnimatorComponent(const AnimatorData& animatorData);

    void Update(float deltaTime) override;
    void DrawImGui() override;
    const char* GetType() const override {
        return "AnimatorComponent";
    }

    std::unique_ptr<Component> Clone() const override;

    void Serialize(std::ofstream& out) const override;
    void Deserialize(std::ifstream& in) override;

private:
    std::string currentAnimationPath;
    Animation currentAnimation;
    int currentFrame = 0;
    float timeAccumulator = 0.0f;
};