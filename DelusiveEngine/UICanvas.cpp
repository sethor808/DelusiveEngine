#include "UICanvas.h"
#include "DelusiveUI.h"
#include "DelusiveRegistry.h"
#include "DelusiveRenderer.h"
#include "UIManager.h"
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>
#include <iostream>

UICanvas::UICanvas(DelusiveRenderer& _renderer)
	: renderer(_renderer), registry(std::make_unique<PropertyRegistry>())
{
	RegisterProperties();
}

UICanvas::~UICanvas() = default;

void UICanvas::RegisterProperties()
{
	registry->Register("name", &name);
}

std::unique_ptr<UICanvas> UICanvas::Clone() const {
	// Create a new canvas with the same name
	std::unique_ptr<UICanvas> copy = std::make_unique<UICanvas>(renderer);
	copy->name = this->name;
	copy->SetActive(this->IsActive());

	// Copy each child element by cloning them
	for (const auto& element : elements) {
		copy->AddElement(std::move(element->Clone()));
	}

	// Copy activation state
	copy->SetActive(this->IsActive());

	return copy;
}

void UICanvas::Update(float deltaTime) {
	if (!active) return;
	for (auto& element : elements) {
		element->Update(deltaTime);
	}
}

void UICanvas::Draw(const glm::mat4& projection) {
	//if (!active) return;
	for (auto& element : elements) {
		element->Draw(projection);
	}
}

void UICanvas::HandleMouse(const glm::vec2& pos, bool down) {
	if (!active) return;
	for (auto& element : elements) {
		element->HandleMouse(pos, down);
	}
}

void UICanvas::HandleInput(const PlayerInputState& input) {
	if (!active) return;
	for (auto& element : elements) {
		element->HandleInput(input);
	}
}

void UICanvas::AddElement(std::unique_ptr<UIElement> element) {
	element->LinkCanvas(this);
	elements.push_back(std::move(element));
}

PlayerAgent* UICanvas::FetchPlayer() const {
	return uiManager ? uiManager->FetchPlayer() : nullptr;
}

void UICanvas::DrawImGui() {
	registry->DrawImGui();

	for (size_t i = 0; i < elements.size(); ++i) {
		ImGui::PushID(static_cast<int>(i));

		if (ImGui::TreeNodeEx("##element", ImGuiTreeNodeFlags_DefaultOpen,
			"[%s] %zu", elements[i]->GetType().c_str(), i)) {
			elements[i]->DrawImGui();

			if (ImGui::Button("Remove Element")) {
				elements.erase(elements.begin() + i);
				ImGui::TreePop();
				ImGui::PopID();
				break;
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	if (ImGui::Button("Add Element")) {
		ImGui::OpenPopup("AddUIElementPopup");
	}

	if (ImGui::BeginPopup("AddUIElementPopup")) {
		std::string type = DelusiveUI::DrawUIElementAddMenu();
		if (!type.empty()) {
			auto newElement = DelusiveUI::CreateUIElementByType(type, renderer);
			if (newElement) {
				AddElement(std::move(newElement));
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void UICanvas::SerializeToFile() const {
	return;
	/*
	if (filePath.empty()) return;
	std::ofstream out(filePath, std::ios::binary);
	if (out) {
		Serialize(out);
	}
	*/
}

std::unique_ptr<UICanvas> UICanvas::LoadFromFile(const std::string& path) {
	std::ifstream in(path, std::ios::binary);
	if (!in) return nullptr;

	auto canvas = std::make_unique<UICanvas>(renderer);
	canvas->Deserialize(in);
	return canvas;
}

void UICanvas::Serialize(std::ostream& out) const {
	out << "[UICanvas]\n";
	registry->Serialize(out);

	// Serialize elements here
	for (auto& elem : elements) {
		elem->Serialize(out);
	}

	out << "[/UICanvas]\n";
}

void UICanvas::Deserialize(std::istream& in) {
	elements.clear();

	std::string line;
	while (std::getline(in, line)) {
		if (line.empty()) continue;

		if (line == "[/UICanvas]") {
			break;
		}

		// Expecting line like: [UIElement UILabel]
		if (line.rfind("[UIElement", 0) == 0) {
			std::istringstream iss(line);
			std::string discard, typeToken;
			iss >> discard >> typeToken; // discard == "[UIElement", typeToken == "UILabel]"

			// strip trailing ']' if present
			if (!typeToken.empty() && typeToken.back() == ']') typeToken.pop_back();

			std::unique_ptr<UIElement> elem = DelusiveUI::CreateUIElementByType(typeToken, renderer);

			if (elem) {
				// Let the element deserialize itself (it will consume until [/UIElement])
				elem->Deserialize(in);
				AddElement(std::move(elem));
			}
			else {
				// Unknown type; skip until [/UIElement]
				while (std::getline(in, line)) {
					if (line == "[/UIElement]") break;
				}
			}
		}
		else {
			std::string key, value;
			auto pos = line.find('=');
			if (pos != std::string::npos) {
				key = line.substr(0, pos);
				value = line.substr(pos + 1);
			}
			else {
				std::istringstream iss(line);
				iss >> key;
				std::getline(iss, value);
				if (!value.empty() && value[0] == ' ') value.erase(0, 1); // trim leading space
			}

			if (key.empty()) continue;

			// Try registry first
			std::istringstream valStream(value);
			bool handled = false;
			for (auto& p : registry->properties) {
				if (p->name == key) {
					p->Deserialize(valStream);
					handled = true;
					break;
				}
			}
			if (handled) continue;
		}
		// If we get here, it's an unexpected line inside canvas; ignore or log.
	}
}

void UICanvas::Reset() {
	elements.clear();
}

void UICanvas::SetActive(bool state) {
	active = state;
}