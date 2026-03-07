#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Scene/UIManager.h>
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

ScriptManager& UICanvas::GetScriptManager() const {
    if (uiManager != nullptr) {
        return uiManager->GetScriptManager();
    }
    throw std::runtime_error("UICanvas::GetScriptManager() - UIManager is null");
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
			auto newElement = DelusiveUI::CreateUIElementByType(type, renderer, uiManager->GetScriptManager());
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

void UICanvas::Deserialize(DelusiveParser::DataBlock& block) {
    elements.clear();

    if (!uiManager) {
        std::cerr << "[UICanvas]::Deserialize called without linking UIManager" << std::endl;
        return;
    }

    registry->Deserialize(block);

    for (auto& child : block.children)
    {
        if (child.category == "UIElement")
        {
            auto element = DelusiveUI::CreateUIElementByType(child.type, renderer, uiManager->GetScriptManager());

            element->LinkCanvas(this);
            element->Deserialize(child);

            elements.push_back(std::move(element));
        }
    }
}

void UICanvas::Deserialize(std::istream& in) {
    //possibly old code
	//std::string line;
	//while (std::getline(in, line)) {
	//	if (line.empty()) continue;

	//	if (line == "[/uicanvas]") {
	//		break;
	//	}

	//	// expecting line like: [uielement uilabel]
	//	if (line.rfind("[uielement", 0) == 0) {
	//		std::istringstream iss(line);
	//		std::string discard, typetoken;
	//		iss >> discard >> typetoken; // discard == "[uielement", typetoken == "uilabel]"

	//		// strip trailing ']' if present
	//		if (!typetoken.empty() && typetoken.back() == ']') typetoken.pop_back();

	//		std::unique_ptr<uielement> elem = delusiveui::createuielementbytype(typetoken, renderer, uimanager->getscriptmanager());

	//		if (elem) {
	//			// let the element deserialize itself (it will consume until [/uielement])
 //               uielement* link = elem.get();
 //               addelement(std::move(elem));
	//			link->deserialize(in);
	//			
	//		}
	//		else {
	//			// unknown type; skip until [/uielement]
	//			while (std::getline(in, line)) {
	//				if (line == "[/uielement]") break;
	//			}
	//		}
	//	}
	//	else {
	//		std::string key, value;
	//		auto pos = line.find('=');
	//		if (pos != std::string::npos) {
	//			key = line.substr(0, pos);
	//			value = line.substr(pos + 1);
	//		}
	//		else {
	//			std::istringstream iss(line);
	//			iss >> key;
	//			std::getline(iss, value);
	//			if (!value.empty() && value[0] == ' ') value.erase(0, 1); // trim leading space
	//		}

	//		if (key.empty()) continue;

	//		// try registry first
	//		std::istringstream valstream(value);
	//		bool handled = false;
	//		for (auto& p : registry->properties) {
	//			if (p->name == key) {
	//				p->deserialize(valstream);
	//				handled = true;
	//				break;
	//			}
	//		}
	//		if (handled) continue;
	//		// if we get here, it's an unexpected line inside canvas; ignore or log.
	//		std::cerr << "unexpected value within [uicanvas]: " << value << std::endl;
	//	}
	//	
	//}
}

void UICanvas::Reset() {
	elements.clear();
}

void UICanvas::SetActive(bool state) {
	active = state;
}