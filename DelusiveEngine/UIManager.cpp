#include "UIManager.h"
#include "DelusiveUIRegistry.h"
#include <iostream>
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>

UIManager::UIManager() {
	name = "NewUIManager";
	activeCanvasName = "";
	activeCanvas = nullptr;
	RegisterProperties();
}

void UIManager::RegisterProperties() {
	SceneSystem::RegisterProperties();
	registry.Register("activeCanvasName", &activeCanvasName);
	registry.Register("canvasList", &canvasList);
}

void UIManager::SetCanvasActive(const std::string& name) {
	if (auto canvas = DelusiveUIRegistry::Instance().Get(name)) {
		canvas->SetActive(true);
		activeCanvasName = name;
		activeCanvas = canvas;
	}
}

void UIManager::Update(float deltaTime) {
	if(activeCanvas && activeCanvas->IsActive()) {
		activeCanvas->Update(deltaTime);
	}
}

void UIManager::Draw(const glm::mat4& projection) {
	if (activeCanvas && activeCanvas->IsActive()) {
		activeCanvas->Draw(projection);
	}
}

void UIManager::HandleMouse(const glm::vec2& mousePos, bool mouseDown) {
	if (activeCanvas && activeCanvas->IsActive()) {
		activeCanvas->HandleMouse(mousePos, mouseDown);
	}
}

void UIManager::DrawImGui() {
    ImGui::Text("UIManager");
    ImGui::Separator();

    // Active canvas selection
    auto allNames = DelusiveUIRegistry::Instance().GetAllNames();
    if (!canvasList.empty()) {
        if (ImGui::BeginCombo("Active Canvas", activeCanvasName.empty() ? "<none>" : activeCanvasName.c_str())) {
            for (const auto& name : canvasList) {
                bool isSelected = (activeCanvasName == name);
                if (ImGui::Selectable(name.c_str(), isSelected)) {
                    SetCanvasActive(name);
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    else {
        ImGui::TextDisabled("No canvases assigned.");
    }

    ImGui::Separator();

    // List assigned canvases
    ImGui::Text("Canvas List:");
    for (size_t i = 0; i < canvasList.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        ImGui::Text("%s", canvasList[i].c_str());
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            if (activeCanvasName == canvasList[i]) {
                Reset(); // active cleared
            }
            canvasList.erase(canvasList.begin() + i);
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }

    // Add canvases (from registry or new)
    if (ImGui::Button("Add Canvas")) {
        ImGui::OpenPopup("AddCanvasPopup");
    }
    if (ImGui::BeginPopup("AddCanvasPopup")) {
        for (const auto& name : allNames) {
            if (std::find(canvasList.begin(), canvasList.end(), name) == canvasList.end()) {
                if (ImGui::MenuItem(name.c_str())) {
                    canvasList.push_back(name);
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        if (ImGui::MenuItem("New Canvas")) {
            std::string newName = "Canvas_" + std::to_string(allNames.size());
            auto newCanvas = std::make_unique<UICanvas>();
            newCanvas->SetName(newName);
            DelusiveUIRegistry::Instance().Register(std::move(newCanvas));
            canvasList.push_back(newName);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();

    // Show the active canvas inspector
    if (activeCanvas) {
        if (ImGui::CollapsingHeader("Active Canvas Inspector")) {
            activeCanvas->DrawImGui();
        }
    }
}

void UIManager::Reset() {
	activeCanvasName.clear();
	activeCanvas = nullptr;
}

std::unique_ptr<SceneSystem> UIManager::Clone() const {
	auto clone = std::make_unique<UIManager>();
	clone->activeCanvasName = activeCanvasName;
	clone->canvasList = canvasList;

	// only refresh pointer if canvas still exists
	if (auto canvas = DelusiveUIRegistry::Instance().Get(activeCanvasName)) {
		clone->activeCanvas = canvas;
	}
	return clone;
}

void UIManager::SaveToFile(std::ofstream& out) const {
    DelusiveUIRegistry::Instance().SaveAll();
	out << "[UIManager]\n";

	out << "ActiveCanvas=" << activeCanvasName << "\n";
	out << "CanvasList=";
	for (size_t i = 0; i < canvasList.size(); ++i) {
		out << canvasList[i];
		if (i < canvasList.size() - 1) out << ",";
	}
	out << "\n";

	out << "[/UIManager]\n";
}

void UIManager::Serialize(std::ostream& out) const {
    SceneSystem::Serialize(out);
	DelusiveUIRegistry::Instance().SaveAll();
}

void UIManager::Deserialize(std::istream& in) {
	canvasList.clear();
	activeCanvasName.clear();

	SceneSystem::Deserialize(in);

	if (!activeCanvasName.empty() || activeCanvasName != "") {
		SetCanvasActive(activeCanvasName);
	}
}