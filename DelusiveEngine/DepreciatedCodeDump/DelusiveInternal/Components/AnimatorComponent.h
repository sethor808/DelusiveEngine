#pragma once
#include <DelusiveInternal/Components/Component.h>
#include <DelusiveInternal/Components/SpriteComponent.h>
#include <DelusiveInternal/Animation/Animation.h>
#include <DelusiveInternal/Animation/AnimatorData.h>

class AnimatorComponent : public Component {
public:
    AnimatorComponent(DelusiveRenderer&);
    AnimatorComponent() = delete;

    void Update(float deltaTime) override;
    void DrawImGui() override;
    const char* GetType() const override {
        return "AnimatorComponent";
    }

    std::unique_ptr<Component> Clone() const override;

    void PlayBranch(const std::string&);
    void ApplyComponentOverrides();

    //Getters & Setters
	void Start() { playing = true; }
	void Stop() { playing = false; }
	bool IsPlaying() const { return playing; }
    void SetAnimatorData(const AnimatorData& animatorData);
	int GetCurrentFrame() const { return currentFrame; }
    float GetTimeAccumulated() const { return timeAccumulator; }
    std::vector<std::string>& GetFlags() { return currentAnimation.data.flags; }

private:
    std::string currentAnimationPath;
    Animation currentAnimation;
    AnimationBranch* currentBranch = nullptr;
    int currentFrame = 0;
    float timeAccumulator = 0.0f;
    bool playing = true;
};