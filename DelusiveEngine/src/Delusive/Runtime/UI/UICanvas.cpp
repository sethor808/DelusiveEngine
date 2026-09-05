#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Scene/UIManager.h>
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>
#include <iostream>

UICanvas::UICanvas(DelusiveInstance& instance)
	: instance(instance), registry(std::make_unique<PropertyRegistry>())
{
	RegisterProperties();
}

UICanvas::~UICanvas() = default;

void UICanvas::RegisterProperties()
{
    registry->category = "Canvas";
	registry->Register("name", &name);
}

std::unique_ptr<UICanvas> UICanvas::Clone() const {
	// Create a new canvas with the same name
	std::unique_ptr<UICanvas> copy = std::make_unique<UICanvas>(instance);
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

std::vector<UIElement*> UICanvas::GetElements() const {
	std::vector<UIElement*> refs;
	refs.reserve(elements.size());

	for (auto& element : elements) {
		refs.push_back(element.get());
	}

	return refs;
}

void UICanvas::AddElement(std::unique_ptr<UIElement> element) {
	element->LinkCanvas(this);

    UUID id = element->GetID();
    idManager.Register(element.get(), id);

	elements.push_back(std::move(element));
}

PlayerAgent* UICanvas::FetchPlayer() const {
	return uiManager ? uiManager->FetchPlayer() : nullptr;
}

void UICanvas::DrawImGui() {
    ImGui::SeparatorText("Canvas");

	registry->DrawImGui();

    ImGui::SeparatorText("Elements");

	for (size_t i = 0; i < elements.size(); ++i) {
        UIElement* element = elements[i].get();

		ImGui::PushID(static_cast<int>(i));

        bool open = ImGui::TreeNodeEx(
            "##element",
            ImGuiTreeNodeFlags_DefaultOpen,
            "[%s] %s",
            element->GetType().c_str(),
            element->GetName().c_str()
        );

        if (open)
        {
            element->DrawImGui();

            if (ImGui::Button("Remove Element"))
            {
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
			auto newElement = DelusiveUI::CreateUIElementByType(type, instance);
			if (newElement) {
				AddElement(std::move(newElement));
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void UICanvas::Reset() {
	elements.clear();
}

void UICanvas::SetActive(bool state) {
	active = state;
}