#include <Delusive/Runtime/Components/DelusiveComponentFactory.h>
#include <Delusive/Runtime/Components/ColliderComponent.h>
#include <Delusive/Runtime/Collision/HitboxCollider.h>
#include <Delusive/Runtime/Collision/HurtboxCollider.h>
#include <Delusive/Runtime/Collision/SolidCollider.h>
#include <Delusive/Runtime/Components/SpriteComponent.h>
#include <Delusive/Runtime/Components/StatsComponent.h>
#include <Delusive/Runtime/Collision/TriggerCollider.h>
#include <Delusive/Runtime/Components/TransformComponent.h>
#include <Delusive/Runtime/Components/AnimatorComponent.h>
#include <Delusive/Runtime/Components/PathfindingComponent.h>
#include <Delusive/Runtime/Components/ScriptComponent.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <imgui/imgui.h>

std::string DelusiveComponentFactory::DrawComponentAddMenu() {
    std::string selected;

        if (ImGui::BeginMenu("Rendering"))
        {
            if (ImGui::MenuItem("SpriteComponent")) {
                selected = "SpriteComponent";
            }
            if (ImGui::MenuItem("AnimatorComponent")) {
                selected = "AnimatorComponent";
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Gameplay"))
        {
            if (ImGui::MenuItem("StatsComponent")) {
                selected = "StatsComponent";
            }

            if (ImGui::MenuItem("ScriptComponent")) {
                selected = "ScriptComponent";
            }

            if (ImGui::MenuItem("PathfindingComponent")) {
                selected = "PathfindingComponent";
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Collision"))
        {
            if (ImGui::MenuItem("SolidCollider")) {
                selected = "SolidCollider";
            }

            if (ImGui::MenuItem("TriggerCollider")) {
                selected = "TriggerCollider";
            }

            if (ImGui::MenuItem("HitboxCollider")) {
                selected = "HitboxCollider";
            }

            if (ImGui::MenuItem("HurtboxCollider")) {
                selected = "HurtboxCollider";
            }

            ImGui::EndMenu();
        }

        return selected;
}

std::unique_ptr<Component> DelusiveComponentFactory::CreateComponentByType(const std::string& type, DelusiveInstance& instance)
{
    if (type == "SpriteComponent") {
        return std::make_unique<SpriteComponent>(instance);
    }
    else if (type == "StatsComponent") {
        return std::make_unique<StatsComponent>(instance);
    }
    else if (type == "AnimatorComponent") {
        return std::make_unique<AnimatorComponent>(instance);
    }
    else if (type == "PathfindingComponent") {
        return std::make_unique<PathfindingComponent>(instance);
    }
    else if (type == "SolidCollider") {
        return std::make_unique<SolidCollider>(instance);
    }
    else if (type == "HitboxCollider") {
        return std::make_unique<HitboxCollider>(instance);
    }
    else if (type == "HurtboxCollider") {
        return std::make_unique<HurtboxCollider>(instance);
    }
    else if (type == "TriggerCollider") {
        return std::make_unique<TriggerCollider>(instance);
    }
    else if (type == "ScriptComponent") {
        return std::make_unique<ScriptComponent>(instance);
    }
    return nullptr;
}