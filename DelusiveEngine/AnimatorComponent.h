#pragma once
#include "Component.h"
#include "SpriteComponent.h"
#include "Animation.h"
#include "AnimatorData.h"

class AnimatorComponent : public Component {
public:
    AnimatorComponent();
    AnimatorComponent(const AnimatorData& animatorData);

    void Update(float deltaTime) override;
    void DrawImGui() override;
    const char* GetType() const override {
        return "AnimatorComponent";
    }

    std::unique_ptr<Component> Clone() const override;

    void PlayBranch(const std::string&);
    void ApplyComponentOverrides();

    //void Serialize(std::ofstream& out) const override;
    //void Deserialize(std::ifstream& in) override;
private:
    std::string currentAnimationPath;
    Animation currentAnimation;
    AnimationBranch* currentBranch = nullptr;
    int currentFrame = 0;
    float timeAccumulator = 0.0f;
    bool playing = true;
};