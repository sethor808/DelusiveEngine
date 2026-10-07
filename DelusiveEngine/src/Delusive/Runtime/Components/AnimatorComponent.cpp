#include <Delusive/Runtime/Components/AnimatorComponent.h>
#include <Delusive/Runtime/Components/SpriteComponent.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Core/DelusiveLibrary.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <imgui/imgui.h>
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <sstream>

using namespace DelusiveAnimation;

AnimatorComponent::AnimatorComponent(DelusiveInstance& instance)
    : Component(instance)
{
    name = "New Animator";
    RegisterProperties();
}

void AnimatorComponent::RegisterProperties() {
    Component::RegisterProperties();
    registry->Register("animationSet", &setID);
}

void AnimatorComponent::Deserialize(DelusiveParser::DataBlock& in) {
    Component::Deserialize(in);
    LinkSet(setID);
}

void AnimatorComponent::SetOwner(Agent* agent) {
    Component::SetOwner(agent);
    ApplyFrame();
}

const Branch* AnimatorComponent::CurrentBranch() const {
    if (currentBranch < 0 || currentBranch >= (int)set.branches.size()) return nullptr;
    return &set.branches[currentBranch];
}

bool AnimatorComponent::LinkSet(const UUID& newSetID) {
    setID = newSetID;
    currentBranch = -1;
    currentFrame = 0;
    tickInFrame = 0;

    //A missing set keeps its id so the link survives a save and can resolve later
    if (!setID.IsValid() || !set.FromLibrary(instance.delusiveLibrary, setID)) {
        set = Set();
        return false;
    }

    if (!set.branches.empty()) {
        currentBranch = 0;
        ApplyFrame();
    }
    return true;
}

void AnimatorComponent::PlayBranch(const UUID& branchID) {
    //By id, not name - names are labels and may repeat
    for (int i = 0; i < (int)set.branches.size(); ++i) {
        if (set.branches[i].id == branchID) {
            currentBranch = i;
            currentFrame = 0;
            tickInFrame = 0;
            playing = true;
            ApplyFrame();
            return;
        }
    }
}

bool AnimatorComponent::SetFlag(const std::string& name, bool value) {
    if (!HasFlag(name)) return false;
    flagValues[name] = value;
    return true;
}

bool AnimatorComponent::Trigger(const std::string& name) {
    if (!HasFlag(name)) return false;
    flagValues[name] = true;
    return true;
}

bool AnimatorComponent::HasFlag(const std::string& name) const {
    return set.FindFlag(name) != nullptr;
}

std::vector<AnimatorComponent::FlagState> AnimatorComponent::GetFlags() const {
    std::vector<FlagState> flags;
    flags.reserve(set.flags.size());
    for (const Flag& flag : set.flags) {
        flags.push_back({ flag.name, flag.kind, GetFlag(flag.name) });
    }
    return flags;
}

bool AnimatorComponent::GetFlag(const std::string& name) const {
    auto it = flagValues.find(name);
    return it != flagValues.end() && it->second;
}

bool AnimatorComponent::IsInputLocked() const {
    const Branch* branch = CurrentBranch();
    if (!branch) return false;
    if (branch->lockInput) return true;
    return currentFrame < (int)branch->frames.size() && branch->frames[currentFrame].inputLock;
}

std::string AnimatorComponent::GetBranchName() const {
    const Branch* branch = CurrentBranch();
    return branch ? branch->name : "";
}

bool AnimatorComponent::Matches(const Transition& t, bool finished, bool fromAny) const {
    if (!t.target.IsValid() || !set.FindBranch(t.target)) return false;
    if (t.onFinish && !finished) return false;

    //Frame windows belong to the branch being left, so they only apply to its own transitions
    if (!fromAny) {
        const int frame = currentFrame + 1;
        if (t.fromFrame > 0 && frame < t.fromFrame) return false;
        if (t.toFrame > 0 && frame > t.toFrame) return false;
    }

    for (const Condition& cond : t.conditions) {
        const Flag* flag = set.FindFlag(cond.flag);
        const bool value = GetFlag(cond.flag);
        if (flag && flag->kind == FlagKind::Trigger) {
            if (!value) return false;
        }
        else if (value != cond.value) {
            return false;
        }
    }
    return true;
}

void AnimatorComponent::Fire(const Transition& t) {
    //A trigger is spent by the transition that used it
    for (const Condition& cond : t.conditions) {
        const Flag* flag = set.FindFlag(cond.flag);
        if (flag && flag->kind == FlagKind::Trigger) flagValues[cond.flag] = false;
    }
    PlayBranch(t.target);
}

void AnimatorComponent::FollowTransitions(bool finished) {
    const Branch* branch = CurrentBranch();

    //Interrupts first - taking damage and the like can happen in any branch, locked or not
    for (const Transition& t : set.anyTransitions) {
        //A held bool must not restart its own branch every tick; a trigger may re-enter
        const bool reenter = std::any_of(t.conditions.begin(), t.conditions.end(), [this](const Condition& c) {
            const Flag* flag = set.FindFlag(c.flag);
            return flag && flag->kind == FlagKind::Trigger;
        });
        if (branch && t.target == branch->id && !reenter) continue;

        if (Matches(t, finished, true)) {
            Fire(t);
            return;
        }
    }

    if (!branch) return;
    for (const Transition& t : branch->transitions) {
        if (Matches(t, finished, false)) {
            Fire(t);
            return;
        }
    }
}

void AnimatorComponent::PlayBranch(const std::string& branchName) {
    for (int i = 0; i < (int)set.branches.size(); ++i) {
        if (set.branches[i].name == branchName) {
            currentBranch = i;
            currentFrame = 0;
            tickInFrame = 0;
            playing = true;
            ApplyFrame();
            return;
        }
    }
}

void AnimatorComponent::Update(float) {
    const Branch* branch = CurrentBranch();
    bool finished = false;

    if (branch && branch->frames.empty()) {
        finished = true; //Nothing to show - pass straight through
    }
    else if (branch && playing && ++tickInFrame >= branch->frames[currentFrame].ticks) {
        tickInFrame = 0;

        if (currentFrame + 1 < (int)branch->frames.size()) {
            currentFrame++;
            ApplyFrame();
        }
        else if (branch->loop) {
            currentFrame = 0;
            finished = true;
            ApplyFrame();
        }
        else {
            //Hold the last frame unless a transition takes over
            playing = false;
            finished = true;
        }
    }

    FollowTransitions(finished);

    //Unused triggers expire - buffering input is the input code's job, not the animator's
    for (const Flag& flag : set.flags) {
        if (flag.kind == FlagKind::Trigger) flagValues[flag.name] = false;
    }
}

void AnimatorComponent::ApplyFrame() {
    const Branch* branch = CurrentBranch();
    if (!branch || currentFrame >= (int)branch->frames.size() || !GetOwner()) return;

    const Frame& frame = branch->frames[currentFrame];
    if (frame.texturePath.empty()) return;

    //The animator owns its sprite's placement: size comes from the image and the set's
    //pixels per unit, and the offset puts the frame's pivot on the agent's position. Image
    //pixels run top-down, world y runs up, hence the flipped y.
    for (SpriteComponent* sprite : GetOwner()->GetComponentsOfType<SpriteComponent>()) {
        sprite->SetTexturePath(frame.texturePath);

        const glm::ivec2 imageSize = instance.renderer.GetTextureSize(frame.texturePath);
        if (imageSize.x > 0 && imageSize.y > 0) {
            const float ppu = set.pixelsPerUnit > 0.0f ? set.pixelsPerUnit : DELUSIVE_PIXEL_SCALE;
            const glm::vec2 pivot = set.PivotFor(frame, imageSize);
            sprite->SetScale(imageSize.x / ppu, imageSize.y / ppu);
            sprite->SetPosition((imageSize.x * 0.5f - pivot.x) / ppu, (pivot.y - imageSize.y * 0.5f) / ppu);
        }
        break;
    }
}

void AnimatorComponent::DrawImGui() {
    Component::DrawImGui();
    ImGui::Separator();

    //Sets are listed from the library, so any indexed .anim can be linked
    auto SetLabel = [this](const DelusiveParser::DataBlock& block) {
        std::string setName = "Unnamed";
        auto prop = block.properties.find("name");
        if (prop != block.properties.end()) {
            std::istringstream in(prop->second);
            in >> std::quoted(setName);
        }
        std::string file = std::filesystem::path(instance.delusiveLibrary.GetSourceFile(block.id)).filename().string();
        return setName + " (" + file + ")";
    };

    const DelusiveParser::DataBlock* linked = instance.delusiveLibrary.Find(setID);
    std::string preview = linked ? SetLabel(*linked) : (setID.IsValid() ? "<missing set>" : "<none>");

    if (ImGui::BeginCombo("Animation Set", preview.c_str())) {
        if (ImGui::Selectable("<none>", !setID.IsValid())) {
            LinkSet(UUID());
        }
        for (const DelusiveParser::DataBlock* block : instance.delusiveLibrary.List("AnimationSet")) {
            ImGui::PushID(block);
            if (ImGui::Selectable(SetLabel(*block).c_str(), block->id == setID)) {
                LinkSet(block->id);
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    if (setID.IsValid()) {
        ImGui::SameLine();
        if (ImGui::Button("Reload")) {
            LinkSet(setID);
        }
    }

    if (ImGui::Button("Delete Component")) {
        MarkToDelete();
    }

    if (set.branches.empty()) {
        ImGui::TextDisabled("No branches");
        return;
    }

    ImGui::Text("Branches:");
    for (int i = 0; i < (int)set.branches.size(); ++i) {
        ImGui::PushID(i);
        if (ImGui::Selectable(set.branches[i].name.c_str(), i == currentBranch)) {
            PlayBranch(set.branches[i].name);
        }
        ImGui::PopID();
    }

    if (ImGui::Button(playing ? "Pause" : "Play")) {
        playing = !playing;
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart")) {
        currentFrame = 0;
        tickInFrame = 0;
        playing = true;
        ApplyFrame();
    }

    if (const Branch* branch = CurrentBranch()) {
        ImGui::Text("Frame: %d / %d", currentFrame + 1, (int)branch->frames.size());
        if (currentFrame < (int)branch->frames.size()) {
            const Frame& frame = branch->frames[currentFrame];
            ImGui::Text("Tick: %d / %d", tickInFrame + 1, frame.ticks);
            ImGui::Text("Input Lock: %s", frame.inputLock ? "Yes" : "No");
            ImGui::Text("Boxes: %d", (int)frame.boxes.size());
        }
    }

    if (!set.flags.empty()) {
        ImGui::Separator();
        ImGui::Text("Flags:");
        for (const Flag& flag : set.flags) {
            ImGui::PushID(flag.name.c_str());
            if (flag.kind == FlagKind::Trigger) {
                if (ImGui::SmallButton(flag.name.c_str())) Trigger(flag.name);
            }
            else {
                bool value = GetFlag(flag.name);
                if (ImGui::Checkbox(flag.name.c_str(), &value)) SetFlag(flag.name, value);
            }
            ImGui::PopID();
        }
    }
}
