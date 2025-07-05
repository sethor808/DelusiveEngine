#include "AnimatorComponent.h"
#include <imgui/imgui.h>
#include <fstream>
#include <iostream>

AnimatorComponent::AnimatorComponent() = default;
AnimatorComponent::AnimatorComponent(const AnimatorData& animatorData)
	: data(animatorData) {}

void AnimatorComponent::Update(float deltaTime) {
	//TODO: Change frame to frame
}

void AnimatorComponent::DrawImGui() {
    ImGui::Text("Animator Component");
    ImGui::Separator();
    ImGui::Text("Branches: %d", (int)data.branches.size());

    // Optional: display current branch/frame status
}


std::unique_ptr<Component> AnimatorComponent::Clone() const {
    return std::make_unique<AnimatorComponent>(data);
}

void AnimatorComponent::Serialize(std::ofstream& out) const {
    if (!out.is_open()) return;

    out << "branches " << data.branches.size() << "\n";
    for (const auto& branch : data.branches) {
        out << "branch " << branch.name << "\n";
        out << "conditions " << branch.conditions.size() << "\n";
        for (const auto& cond : branch.conditions) {
            out << "condition " << cond.flag << " " << cond.expectedValue << "\n";
        }
        out << "frames " << branch.frames.size() << "\n";
        for (const auto& frame : branch.frames) {
            out << "frame " << frame.flagChanges.size() << " " << frame.duration << "\n";
            for (const auto& flag : frame.flagChanges) {
                out << flag.flag << " " << flag.set << " " << flag.duration << "\n";
            }
        }
    }
}


void AnimatorComponent::Deserialize(std::ifstream& in) {
    if (!in.is_open()) return;

    data.branches.clear();
    std::string token;
    size_t branchCount = 0;
    in >> token >> branchCount;

    for (size_t b = 0; b < branchCount; ++b) {
        AnimationBranch branch;
        in >> token >> branch.name;

        size_t condCount = 0;
        in >> token >> condCount;
        for (size_t c = 0; c < condCount; ++c) {
            std::string flag;
            bool expected;
            in >> token >> flag >> expected;
            branch.conditions.push_back({ flag, expected });
        }

        size_t frameCount = 0;
        in >> token >> frameCount;
        for (size_t f = 0; f < frameCount; ++f) {
            AnimationFrame frame;
            size_t flagCount = 0;
            in >> token >> flagCount >> frame.duration;

            for (size_t fc = 0; fc < flagCount; ++fc) {
                std::string flag;
                bool set;
                float duration;
                in >> flag >> set >> duration;
                frame.flagChanges.push_back({ flag, set, duration });
            }

            branch.frames.push_back(frame);
        }

        data.branches.push_back(branch);
    }
}