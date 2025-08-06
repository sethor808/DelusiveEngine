#include "UICanvas.h"
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>
#include <iostream>
/*
UICanvas::UICanvas(const UICanvas& other) {
	name = other.name;
	active = other.active;
	for (const auto& element : other.elements) {
		elements.push_back(std::move(element->Clone()));
	}
}

UICanvas& UICanvas::operator=(const UICanvas& other) {
	if (this != &other) {
		active = other.active;
		name = other.name;
		elements.clear();
		for (const auto& element : other.elements) {
			elements.push_back(std::move(element->Clone()));
		}
	}
	return *this;
}
*/

std::unique_ptr<UICanvas> UICanvas::Clone() const {
	// Create a new canvas with the same name
	std::unique_ptr<UICanvas> copy = std::make_unique<UICanvas>();
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
	if (!active) return;
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
	elements.push_back(std::move(element));
}

void UICanvas::DrawImGui() {
	ImGui::Text("Canvas Name: %s", name.c_str());
	ImGui::Checkbox("Active", &active);
	ImGui::Text("File Path: %s", filePath.empty() ? "(unsaved)" : filePath.c_str());

	if (ImGui::CollapsingHeader("Elements", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (size_t i = 0; i < elements.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));
			if (ImGui::TreeNode("Element", "Element %zu", i)) {
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
	}

	if (ImGui::Button("Add New Element")) {
		// TODO: UI to select type and create a new element
		// Example:
		// AddElement(std::make_unique<UILabel>("New Label"));
	}
}

void UICanvas::SerializeToFile() const {
	if (filePath.empty()) return;
	std::ofstream out(filePath, std::ios::binary);
	if (out) {
		Serialize(out);
	}
}

std::unique_ptr<UICanvas> UICanvas::LoadFromFile(const std::string& path) {
	std::ifstream in(path, std::ios::binary);
	if (!in) return nullptr;

	auto canvas = std::make_unique<UICanvas>();
	canvas->Deserialize(in);
	canvas->filePath = path;
	return canvas;
}

void UICanvas::Serialize(std::ostream& out) const {
	out << "name " << name << "\n";
	out << "filepath " << filePath << "\n";
	out << "active " << (active ? 1 : 0) << "\n";

	out << "elements " << elements.size() << "\n";
	for (const auto& elem : elements) {
		out << "element {\n";
		out << "type " << elem->GetTypeName() << "\n";  // e.g., "UIButton", "UILabel"
		elem->Serialize(out);
		out << "}\n";
	}

	out << "---\n";
}

void UICanvas::Deserialize(std::istream& in) {
	elements.clear();
	std::string line;

	while (std::getline(in, line)) {
		if (line == "---") break;

		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "name") {
			std::getline(iss >> std::ws, name); // grab rest of the line
		}
		else if (token == "filepath") {
			std::getline(iss >> std::ws, filePath);
		}
		else if (token == "active") {
			int val;
			iss >> val;
			active = val != 0;
		}
		else if (token == "elements") {
			size_t count;
			iss >> count;
			elements.reserve(count);
		}
		else if (token == "element") {
			// Read until closing '}'
			std::string type;
			while (std::getline(in, line) && line != "}") {
				std::istringstream subIss(line);
				std::string subToken;
				subIss >> subToken;

				if (subToken == "type") {
					subIss >> type;
				}
				else {
					// Let element itself handle remaining content
					if (!type.empty()) {
						auto element = CreateUIElementByType(type);
						if (element) {
							// let it read all its data until it sees '}'
							element->Deserialize(in);
							elements.push_back(std::move(element));
						}
						break;  // Done with this element
					}
				}
			}
		}
	}
}

void UICanvas::Reset() {
	elements.clear();
}

void UICanvas::SetActive(bool state) {
	active = state;
}