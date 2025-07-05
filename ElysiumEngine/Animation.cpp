#include "Animation.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool Animation::SaveToFile(const std::string& path) const {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "[Animation] Failed to write file: " << path << std::endl;
        return false;
    }

    out << "name " << name << "\n";

    // Save flags
    for (const auto& flag : data.flags) {
        out << "flagdef " << flag << "\n";
    }

    // Save defaultState (optional frame)
    out << "defaultstate\n";
    for (const auto& mod : data.defaultState.componentOverrides) {
        out << "compmod " << mod.componentName << " " << mod.enabled << " "
            << mod.positionOffset.x << " " << mod.positionOffset.y << " "
            << mod.scale.x << " " << mod.scale.y << " " << mod.rotation << "\n";
    }

    // Save branches
    for (const auto& branch : data.branches) {
        out << "branch " << branch.name << "\n";
        out << "loop " << branch.loop << "\n";
        out << "lockinput " << branch.lockInput << "\n";

        for (const auto& cond : branch.conditions) {
            out << "condition " << cond.flag << " " << cond.expectedValue << "\n";
        }

        for (const auto& frame : branch.frames) {
            out << "frame " << frame.duration << " " << frame.inputLock << "\n";

            for (const auto& flag : frame.flagChanges) {
                out << "flag " << flag.flag << " " << flag.set << " " << flag.duration << "\n";
            }

            for (const auto& mod : frame.componentOverrides) {
                out << "compmod " << mod.componentName << " " << mod.enabled << " "
                    << mod.positionOffset.x << " " << mod.positionOffset.y << " "
                    << mod.scale.x << " " << mod.scale.y << " " << mod.rotation << "\n";
            }
        }
    }

    return true;
}

bool Animation::LoadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "[Animation] Failed to open file: " << path << std::endl;
        return false;
    }

    std::string line;
    data.branches.clear();
    data.flags.clear();
    data.defaultState.componentOverrides.clear();
    AnimationBranch* currentBranch = nullptr;
    AnimationFrame* currentFrame = nullptr;
    bool inDefaultState = false;

    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string word;
        iss >> word;

        if (word == "name") {
            iss >> name;
        }
        else if (word == "flagdef") {
            std::string flagName;
            iss >> flagName;
            data.flags.push_back(flagName);
        }
        else if (word == "defaultstate") {
            inDefaultState = true;
            currentBranch = nullptr;
            currentFrame = nullptr;
        }
        else if (word == "branch") {
            std::string branchName;
            iss >> branchName;
            data.branches.push_back({ branchName });
            currentBranch = &data.branches.back();
            currentFrame = nullptr;
            inDefaultState = false;
        }
        else if (word == "loop" && currentBranch) {
            iss >> currentBranch->loop;
        }
        else if (word == "lockinput" && currentBranch) {
            iss >> currentBranch->lockInput;
        }
        else if (word == "condition" && currentBranch) {
            std::string flag;
            bool expected;
            iss >> flag >> expected;
            currentBranch->conditions.push_back({ flag, expected });
        }
        else if (word == "frame" && currentBranch) {
            float duration = 0.1f;
            bool inputLock = false;
            iss >> duration >> inputLock;
            currentBranch->frames.emplace_back();
            currentFrame = &currentBranch->frames.back();
            currentFrame->duration = duration;
            currentFrame->inputLock = inputLock;
        }
        else if (word == "flag" && currentFrame) {
            std::string flag;
            bool set;
            float duration;
            iss >> flag >> set >> duration;
            currentFrame->flagChanges.push_back({ flag, set, duration });
        }
        else if (word == "compmod") {
            ComponentMod mod;
            iss >> mod.componentName >> mod.enabled
                >> mod.positionOffset.x >> mod.positionOffset.y
                >> mod.scale.x >> mod.scale.y >> mod.rotation;

            if (inDefaultState) {
                data.defaultState.componentOverrides.push_back(mod);
            }
            else if (currentFrame) {
                currentFrame->componentOverrides.push_back(mod);
            }
        }
    }

    return true;
}