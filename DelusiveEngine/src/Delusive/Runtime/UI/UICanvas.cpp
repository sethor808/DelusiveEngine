#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Core/DelusiveClone.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Scene/UIManager.h>
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Core/DelusiveLibrary.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <imgui/imgui.h>
#include <filesystem>
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
    registry->category = "UICanvas";
	registry->Register("id", &id);
	registry->Register("name", &name);
}

#pragma region File IO

void UICanvas::Serialize(DelusiveParser::DataBlock& out) const {
	registry->Serialize(out);

	//Ownership is a UUID list - each element writes its own block
	std::ostringstream ids;
	for (const auto& element : elements) {
		if (element) ids << element->GetID().ToString() << " ";
	}
	out.properties["elements"] = ids.str();
}

void UICanvas::Deserialize(DelusiveParser::DataBlock& in) {
	registry->Deserialize(in);
	registry->Resolve(instance);

	elements.clear();

	auto list = in.properties.find("elements");
	if (list == in.properties.end()) return;

	std::istringstream ids(list->second);
	std::string idText;

	while (ids >> idText) {
		UUID elementID;
		elementID.FromString(idText);

		const DelusiveParser::DataBlock* recipe = instance.delusiveLibrary.Find(elementID);
		if (!recipe) {
			std::cerr << "[UICanvas] Missing element recipe " << idText << " in " << name << std::endl;
			continue;
		}

		std::unique_ptr<UIElement> element = DelusiveBuild<UIElement>(*recipe, instance);
		if (!element) continue;

		AddElement(std::move(element));
	}
}

void UICanvas::CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const {
	DelusiveParser::DataBlock self;
	Serialize(self);
	self.id = id;
	out.push_back(std::move(self));

	for (const auto& element : elements) {
		if (element) element->CollectBlocks(out);
	}
}

std::unique_ptr<UICanvas> UICanvas::FromRecipe(const DelusiveParser::DataBlock& recipe, DelusiveInstance& instance) {
	return DelusiveBuild<UICanvas>(recipe, instance);
}

std::unique_ptr<UICanvas> UICanvas::Clone() const {
	return DelusiveClone<UICanvas>(*this, instance);
}

std::unique_ptr<UICanvas> UICanvas::LoadFromFile(const std::string& path, DelusiveInstance& instance) {
	if (!instance.delusiveLibrary.LoadFile(path)) return nullptr;

	for (const DelusiveParser::DataBlock* block : instance.delusiveLibrary.ListFile(path)) {
		if (block->category == "UICanvas") {
			return FromRecipe(*block, instance);
		}
	}

	std::cerr << "[UICanvas] No UICanvas block in " << path << std::endl;
	return nullptr;
}

bool UICanvas::SaveToFile(const std::string& path, bool* remapped) {
	if (remapped) *remapped = false;
	if (!id.IsValid()) id = UUID::GenerateRandom();

	std::vector<DelusiveParser::DataBlock> blocks;
	CollectBlocks(blocks);

	DelusiveLibrary::IDRemap remap;
	if (!instance.delusiveLibrary.WriteFile(path, std::move(blocks), &remap)) return false;

	if (remapped) *remapped = !remap.empty();
	return true;
}

bool UICanvas::Save(bool* remapped) {
	std::string path = instance.delusiveLibrary.GetSourceFile(id);

	//A new canvas must not land on another canvas's file
	if (path.empty()) {
		const std::string base = std::string(CANVAS_PATH) + name;
		path = base + CANVAS_EXT;
		for (int n = 2; std::filesystem::exists(path); ++n) {
			path = base + "_" + std::to_string(n) + CANVAS_EXT;
		}
	}

	return SaveToFile(path, remapped);
}

#pragma endregion

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