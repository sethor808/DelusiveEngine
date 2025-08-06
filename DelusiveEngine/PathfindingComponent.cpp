#include "PathfindingComponent.h"
#include "PathfindingSystem.h"
#include <glm/glm.hpp>
#include <fstream>

PathfindingComponent::PathfindingComponent() {

}

std::unique_ptr<Component> PathfindingComponent::Clone() const{
	return std::make_unique<PathfindingComponent>(*this);
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

void PathfindingComponent::Serialize(std::ofstream& out) const {
    out << "enabled " << (enabled ? 1 : 0) << "\n";
    out << "debugcolor "
        << debugColor.r << " " << debugColor.g << " "
        << debugColor.b << " " << debugColor.a << "\n";

    out << "---\n";
}

void PathfindingComponent::Deserialize(std::ifstream& in) {
    std::string line;
    while (std::getline(in, line)) {
        if (line == "---") break;

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "enabled") {
            int val;
            iss >> val;
            enabled = val != 0;
        }
        else if (token == "debugcolor") {
            iss >> debugColor.r >> debugColor.g >> debugColor.b >> debugColor.a;
        }
    }
}