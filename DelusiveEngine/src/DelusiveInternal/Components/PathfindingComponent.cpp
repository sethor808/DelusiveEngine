#include <Delusive/Runtime/Components/PathfindingComponent.h>
#include <Delusive/Runtime/Scene/PathfindingSystem.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

PathfindingComponent::PathfindingComponent(DelusiveRenderer& renderer)
    : Component(renderer)
{

}

std::unique_ptr<Component> PathfindingComponent::Clone() const{
    //TODO: Make a proper deep copy
	return std::make_unique<PathfindingComponent>(renderer);
}

void PathfindingComponent::Update(float deltaTime) {

}

void PathfindingComponent::DrawImGui() {
    ImGui::Text("Pathfinding Component");
    ImGui::Separator();

    if (ImGui::ColorEdit4("Debug Color", glm::value_ptr(debugColor))) {
        // Color changed
    }

    ImGui::Text("Path Points: %zu", currentPath.size());
    if (!currentPath.empty()) {
        for (size_t i = 0; i < currentPath.size(); ++i) {
            ImGui::Text(" - (%.2f, %.2f)", currentPath[i].x, currentPath[i].y);
        }
    }

    if (ImGui::Button("Clear Path")) {
        currentPath.clear();
    }

    if (ImGui::Button("Delete Component")) {
        MarkToDelete();
    }
}

void PathfindingComponent::RequestPath(glm::vec2 start, glm::vec2 end) {
    //currentPath = pathfindingSystem.FindPath(start, end);
}