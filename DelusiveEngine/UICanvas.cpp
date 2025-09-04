#include "UICanvas.h"
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>
#include <iostream>

void UICanvas::RegisterProperties() {
	registry.Register("name", &name);
	registry.Register("filepath", &filePath);
}

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
	elements.push_back(std::move(element));
}

void UICanvas::DrawImGui() {
	registry.DrawImGui();

	for (size_t i = 0; i < elements.size(); ++i) {
		ImGui::PushID(static_cast<int>(i));

		if (ImGui::TreeNodeEx("##element", ImGuiTreeNodeFlags_DefaultOpen,
			"[%s] %zu", elements[i]->GetType(), i)) {
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
		ImGui::OpenPopup("AddElementPopup");
	}

	if (ImGui::BeginPopup("AddElementPopup")) {
		if (ImGui::MenuItem("UILabel")) {
			elements.push_back(std::make_unique<UILabel>("New Label"));
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("UIImage")) {
			elements.push_back(std::make_unique<UIImage>());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("UIButton")) {
			elements.push_back(std::make_unique<UIButton>());
			ImGui::CloseCurrentPopup();
		}
		if (ImGui::MenuItem("UIPanel")) {
			elements.push_back(std::make_unique<UIPanel>());
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
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
	out << "[UICanvas]\n";
	registry.Serialize(out);

	// Serialize elements here
	for (auto& elem : elements) {
		elem->Serialize(out);
	}

	out << "[/UICanvas]\n";
}

void UICanvas::Deserialize(std::istream& in) {
	elements.clear();
	registry.Deserialize(in);

	std::string line;
	while (std::getline(in, line)) {
		if (line == "[/UICanvas]") break;

		if (line == "[UILabel]") {
			auto lbl = std::make_unique<UILabel>("");
			lbl->Deserialize(in);
			elements.push_back(std::move(lbl));
		}
		else if (line == "[UIImage]") {
			auto img = std::make_unique<UIImage>();
			img->Deserialize(in);
			elements.push_back(std::move(img));
		}
		else if (line == "[UIButton]") {
			auto btn = std::make_unique<UIButton>();
			btn->Deserialize(in);
			elements.push_back(std::move(btn));
		}
		else if (line == "[UIPanel]") {
			auto pnl = std::make_unique<UIPanel>();
			pnl->Deserialize(in);
			elements.push_back(std::move(pnl));
		}
	}
}

void UICanvas::Reset() {
	elements.clear();
}

void UICanvas::SetActive(bool state) {
	active = state;
}