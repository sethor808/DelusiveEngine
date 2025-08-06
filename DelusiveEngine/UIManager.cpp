#include "UIManager.h"
#include <iostream>
#include <imgui/imgui.h>

UIManager::UIManager() {
	name = "NewUIManager";
}

void UIManager::SetCanvasActive(const std::string& name) {
	for (auto& canvas : canvases) {
		if (canvas->GetName() == name) {
			canvas->SetActive(true);
		}
	}
}

void UIManager::Update(float deltaTime) {
	for (auto& canvas : canvases) {
		if (canvas->IsActive()) {
			canvas->Update(deltaTime);
		}
	}
}

void UIManager::Draw(const glm::mat4& projection) {
	for (auto& canvas : canvases) {
		if (canvas->IsActive()) {
			canvas->Draw(projection);
		}
	}
}

void UIManager::HandleMouse(const glm::vec2& mousePos, bool mouseDown) {
	for (auto& canvas : canvases) {
		if (canvas->IsActive()) {
			canvas->HandleMouse(mousePos, mouseDown);
		}
	}
}

void UIManager::DrawImGui() {
	ImGui::Begin("UI Manager");

	if (ImGui::CollapsingHeader("Canvases", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (size_t i = 0; i < canvases.size(); ++i) {
			auto& canvas = canvases[i];

			ImGui::PushID(static_cast<int>(i));
			bool isActive = canvas->IsActive();
			if (ImGui::Checkbox("##active", &isActive)) {
				canvas->SetActive(isActive);
			}
			ImGui::SameLine();

			ImGui::Text("%s", canvas->GetName().c_str());
			ImGui::SameLine();

			if (ImGui::BeginCombo("##Options", "Edit")) {
				if (ImGui::Selectable("Set Active Only")) {
					SetCanvasActive(canvas->GetName());
				}
				if (ImGui::Selectable("Reset")) {
					canvas->Reset();
				}
				if (ImGui::Selectable("Remove")) {
					canvases.erase(canvases.begin() + i);
					ImGui::EndCombo();
					ImGui::PopID();
					break;
				}
				ImGui::EndCombo();
			}

			ImGui::PopID();
		}
	}

	if (ImGui::Button("Add Canvas")) {
		std::string newName = "Canvas_" + std::to_string(canvases.size());
		auto newCanvas = std::make_unique<UICanvas>(newName);
		canvases.push_back(std::move(newCanvas));
	}

	ImGui::End();
}


void UIManager::Reset() {
	for (auto& canvas : canvases) {
		canvas->Reset();
		canvas->SetActive(false);
	}
}

std::unique_ptr<SceneSystem> UIManager::Clone() const {
	auto clone = std::make_unique<UIManager>();
	for (const auto& canvas : canvases) {
		if (canvas) {
			clone->canvases.push_back(std::move(canvas->Clone())); // calls UICanvas::Clone(), returns unique_ptr<UICanvas>
		}
	}
	return clone;
}

void UIManager::Serialize(std::ostream& out) const {
	out << canvases.size() << "\n";
	for (const auto& canvas : canvases) {
		out << canvas->GetName() << "\n";
		out << canvas->IsActive() << "\n";
		canvas->Serialize(out);
	}
}

void UIManager::Deserialize(std::istream& in) {
	size_t count = 0;
	in >> count;
	in.ignore(); // skip newline

	canvases.clear();
	for (size_t i = 0; i < count; ++i) {
		std::string name;
		std::getline(in, name);

		std::string activeStr;
		std::getline(in, activeStr);
		bool active = (activeStr == "1" || activeStr == "true");

		auto canvas = std::make_unique<UICanvas>();
		canvas->SetName(name);
		canvas->SetActive(active);
		canvas->Deserialize(in);

		canvases.push_back(std::move(canvas));
	}
}