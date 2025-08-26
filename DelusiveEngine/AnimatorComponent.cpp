#include "AnimatorComponent.h"
#include "Agent.h"
#include <imgui/imgui.h>
#include <fstream>
#include <iostream>
#include <filesystem>

AnimatorComponent::AnimatorComponent() = default;
AnimatorComponent::AnimatorComponent(const AnimatorData& animatorData)
    : currentAnimation(animatorData) {
    SetName("NewAnimatorComponent");
}

void AnimatorComponent::Update(float deltaTime) {
    if (!playing || !currentBranch || currentBranch->frames.empty()) {
        return;
    }

    timeAccumulator += deltaTime;
    AnimationFrame& frame = currentBranch->frames[currentFrame];

    if (timeAccumulator >= frame.duration) {
        timeAccumulator -= frame.duration;
        currentFrame++;

        if (currentFrame >= (int)currentBranch->frames.size()) {
            if (currentBranch->loop) {
                currentFrame = 0;
            }
            else {
                currentFrame = (int)currentBranch->frames.size() - 1;
                playing = false;
            }
        }

        // Optional: mark frame dirty for editor refresh, probably remove later
        currentBranch->frames[currentFrame].dirty = true;
    }

    ApplyComponentOverrides();
}

void AnimatorComponent::ApplyComponentOverrides() {
    if (!currentBranch || currentFrame >= currentBranch->frames.size()) {
        return;
    }

    /*
    if (baseAgent) {
        GetOwner()->CopyFrom(*baseAgent);
    }
    */

    const AnimationFrame& frame = currentBranch->frames[currentFrame];

    for (const ComponentMod& mod : frame.componentOverrides) {
        Component* comp = GetOwner()->GetComponentByID(mod.componentID);
        if (!comp) continue;

        comp->SetEnabled(mod.enabled);
        comp->transform.position = mod.positionOffset; // No += here
        comp->transform.scale = mod.scale;
        comp->transform.rotation = mod.rotation;

        if (!mod.texturePath.empty()) {
            comp->SetTexturePath(mod.texturePath);
        }
    }
}

void AnimatorComponent::PlayBranch(const std::string& branchName) {
    for (AnimationBranch& branch : currentAnimation.data.branches) {
        if (branch.name == branchName) {
            currentBranch = &branch;
            currentFrame = 0;
            timeAccumulator = 0.0f;
            playing = true;
            break;
        }
    }
}

void AnimatorComponent::DrawImGui() {
    ImGui::Text("Animator Component");
    ImGui::Separator();

    ImGui::Text("Animation File:");
    ImGui::SameLine();
    ImGui::Text("%s", currentAnimationPath.empty() ? "[None]" : currentAnimationPath.c_str());

    if (ImGui::Button("Change Animation")) {
        ImGui::OpenPopup("AnimationFileBrowser");
    }

    if (ImGui::BeginPopup("AnimationFileBrowser")) {
        for (const auto& entry : std::filesystem::directory_iterator("assets/animations")) { //TODO: Setup a macro path somewhere
            if (entry.is_regular_file() && entry.path().extension() == ".anim") {
                std::string fullPath = entry.path().string();
                std::string filename = entry.path().filename().string();
                if (ImGui::Selectable(filename.c_str())) {
                    currentAnimationPath = fullPath;

                    if (currentAnimation.LoadFromFile(currentAnimationPath)) {
                        currentAnimation.data = currentAnimation.data;
                        if (!currentAnimation.data.branches.empty()) {
                            PlayBranch(currentAnimation.data.branches[0].name);
                        }
                        ImGui::CloseCurrentPopup();
                    }
                    else {
                        std::cerr << "[AnimatorComponent] Failed to load .anim: " << fullPath << "\n";
                    }
                }
            }
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();

    // Branch Selector
    static int selectedBranch = 0;
    ImGui::Text("Animation Branch:");
    for (int i = 0; i < currentAnimation.data.branches.size(); ++i) {
        std::string label = currentAnimation.data.branches[i].name + (i == selectedBranch ? " (current)" : "");
        if (ImGui::Selectable(label.c_str(), i == selectedBranch)) {
            selectedBranch = i;
            PlayBranch(currentAnimation.data.branches[i].name);
        }
    }

    // Playback Controls
    if (ImGui::Button(playing ? "Pause" : "Play")) {
        playing = !playing;
    }
    ImGui::SameLine();
    if (ImGui::Button("Restart")) {
        currentFrame = 0;
        timeAccumulator = 0.0f;
    }

    // Branch Info
    if (currentBranch) {
        ImGui::Text("Branch: %s", currentBranch->name.c_str());
        ImGui::Text("Frame: %d / %d", currentFrame, (int)currentBranch->frames.size() - 1);
        if (currentFrame >= 0 && currentFrame < currentBranch->frames.size()) {
            const AnimationFrame& frame = currentBranch->frames[currentFrame];
            ImGui::Text("Duration: %.2fs", frame.duration);
            ImGui::Text("Input Lock: %s", frame.inputLock ? "Yes" : "No");
            ImGui::Text("Flags: %d, Mods: %d",
                (int)frame.flagChanges.size(),
                (int)frame.componentOverrides.size());
        }
    }
    else {
        ImGui::Text("No branch playing.");
    }

    ImGui::Separator();
    ImGui::Text("Flags:");
    for (const std::string& flag : currentAnimation.data.flags) {
        ImGui::BulletText("%s", flag.c_str());
    }

    if (ImGui::Button("Delete Component")) {
        MarkToDelete();
    }
}


std::unique_ptr<Component> AnimatorComponent::Clone() const {
    auto clone = std::make_unique<AnimatorComponent>(currentAnimation.data);
    clone->currentAnimationPath = currentAnimationPath;
    clone->currentAnimation = currentAnimation;
    return clone;
}

/*
void AnimatorComponent::Serialize(std::ofstream& out) const {
    if (!out.is_open()) return;

    out << "name " << name << "\n";
    out << "animpath " << currentAnimationPath << "\n";
    out << "---\n";
}

void AnimatorComponent::Deserialize(std::ifstream& in) {
    if (!in.is_open()) return;

    std::string line;
    while (std::getline(in, line)) {
        if (line == "---") break;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "name") {
            std::string tempName;
            std::getline(iss, tempName);
            if (!tempName.empty() && tempName[0] == ' ') tempName.erase(0, 1);
            SetName(tempName.c_str());
        }
        else if (token == "animpath") {
            iss >> currentAnimationPath;

            if (!currentAnimation.LoadFromFile(currentAnimationPath)) {
                std::cerr << "[AnimatorComponent] Failed to load .anim file: " << currentAnimationPath << "\n";
                return;
            }

            //data = currentAnimation.data;

            if (!currentAnimation.data.branches.empty()) {
                PlayBranch(currentAnimation.data.branches[0].name);
            }
        }
        else {
            break;
        }
    }
}
*/