#pragma once
#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Animation/AnimationSet.h>
#include <unordered_map>

//Plays the owner's AnimationSet, linked by UUID and resolved through the library.
//Update advances one tick - gameplay runs at a fixed DELUSIVE_TICKS_PER_SECOND - then
//follows the set's transitions using the flags gameplay has set this tick.
class AnimatorComponent : public Component {
public:
    AnimatorComponent(DelusiveInstance&);
    AnimatorComponent() = delete;

    void Update(float deltaTime) override;
    void DrawImGui() override;
    const char* GetType() const override {
        return "AnimatorComponent";
    }

    void RegisterProperties() override;
    void Deserialize(DelusiveParser::DataBlock& in) override;
    //Applies the current frame once there is a sprite to apply it to
    void SetOwner(Agent*) override;

    //Rebuilds the set from the library, which is also how saved .anim edits show up
    bool LinkSet(const UUID& setID);
    void PlayBranch(const std::string&);
    void PlayBranch(const UUID&);
    void ApplyFrame();

    //Gameplay drives branches through flags; the animator never reads input itself.
    //A trigger lasts one tick unless a transition consumes it sooner. Both return false for
    //a flag the set does not declare, so a misspelt name shows up instead of doing nothing.
    bool SetFlag(const std::string& name, bool value);
    bool Trigger(const std::string& name);
    bool GetFlag(const std::string& name) const;

    //What a controller can drive: every flag the set declares, with its current value
    struct FlagState {
        std::string name;
        DelusiveAnimation::FlagKind kind;
        bool value;
    };
    std::vector<FlagState> GetFlags() const;
    bool HasFlag(const std::string& name) const;
    //True while the branch or current frame locks input - for the input code to honour
    bool IsInputLocked() const;
    std::string GetBranchName() const;

    //Getters & Setters
	void Start() { playing = true; }
	void Stop() { playing = false; }
	bool IsPlaying() const { return playing; }
	int GetCurrentFrame() const { return currentFrame; }
    int GetTickInFrame() const { return tickInFrame; }
    UUID GetSetID() const { return setID; }
    const DelusiveAnimation::Set& GetSet() const { return set; }

private:
    const DelusiveAnimation::Branch* CurrentBranch() const;
    //finished: the branch ended (or completed a loop) this tick
    void FollowTransitions(bool finished);
    bool Matches(const DelusiveAnimation::Transition&, bool finished, bool fromAny) const;
    void Fire(const DelusiveAnimation::Transition&);

    std::unordered_map<std::string, bool> flagValues;

    UUID setID;
    DelusiveAnimation::Set set;
    //Index, not pointer - relinking rebuilds the branch list
    int currentBranch = -1;
    int currentFrame = 0;
    int tickInFrame = 0;
    bool playing = true;
};
