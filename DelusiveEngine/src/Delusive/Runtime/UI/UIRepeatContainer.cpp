#define DELUSIVE_EDITOR_MODE
#include <Delusive/Runtime/UI/UIRepeatContainer.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <Delusive/Runtime/Core/DelusiveClone.h>

UIRepeatContainer::UIRepeatContainer(DelusiveInstance& instance)
	: UIElement(instance)
{
	name = "New UIRepeatContainer";
	RegisterProperties();
}

void UIRepeatContainer::RegisterProperties() {
	UIElement::RegisterProperties();
	registry->Register("count", &count);
	registry->Register("rows", &rows);
	registry->Register("spacing", &spacing);
	registry->Register("prototype", &prototype);
	registry->Hide("prototype"); //Edited in the Prototype section below
	registry->Register("source", &source);
	registry->Hide("source");
	registry->Register("bindCount", &bindCount);
}

void UIRepeatContainer::Update(float deltaTime) {
	if (!source.empty() && parentCanvas) {
		std::vector<UIBindItem> items = DelusiveUI::FetchBindSource(source, *parentCanvas);

		//Only rebuild when the data changed, e.g. a talisman took a hit
		if (items != boundItems) {
			boundItems = std::move(items);
			SetCount(static_cast<int>(boundItems.size()));
			RegenerateChildren();

			for (size_t i = 0; i < children.size() && i < boundItems.size(); ++i) {
				children[i]->ApplyBinding(boundItems[i]);
			}
		}
	}

	UIElement::Update(deltaTime);
}

void UIRepeatContainer::ApplyBinding(const UIBindItem& item) {
	if (const std::string* value = DelusiveUI::FetchBindField(item, bindCount)) {
		SetCount(std::atoi(value->c_str()));
		RegenerateChildren();
	}

	UIElement::ApplyBinding(item);
}

void UIRepeatContainer::Draw(const glm::mat4& projection) {
	if (!enabled) return;
	UIElement::Draw(projection);

#ifdef DELUSIVE_EDITOR_MODE
	//Bound containers fill themselves, so their prototype ghost would show in game
	if (prototype && source.empty() && bindCount.empty()) {
		instance.renderer.PushAlpha(0.3f);
		prototype->Draw(projection);
		instance.renderer.PopAlpha();
	}
#endif
}

void UIRepeatContainer::DrawImGui() {
	UIElement::DrawImGui();

	ImGui::SeparatorText("Binding");
	if (ImGui::BeginCombo("source", source.empty() ? "<None>" : source.c_str())) {
		if (ImGui::Selectable("<None>", source.empty())) source.clear();
		for (const std::string& option : DelusiveUI::ListBindSources()) {
			if (ImGui::Selectable(option.c_str(), option == source)) source = option;
		}
		ImGui::EndCombo();
	}

	// --- Prototype editing section ---
	// --- Prototype Management Section ---
	ImGui::SeparatorText("Prototype");

	if (!prototype) {
		ImGui::TextDisabled("No prototype assigned.");

		if (ImGui::Button("Create Prototype", ImVec2(-FLT_MIN, 0))) {
			ImGui::OpenPopup("AddPrototypePopup");
		}

		if (ImGui::BeginPopup("AddPrototypePopup")) {
			std::string type = DelusiveUI::DrawUIElementAddMenu();
			if (!type.empty()) {
				auto newProto = DelusiveUI::CreateUIElementByType(type, instance);
				if (newProto) {
					newProto->LinkCanvas(parentCanvas);
					prototype = std::move(newProto);
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
	else {
		// Collapsible framed region for the prototype
		if (ImGui::TreeNodeEx("Prototype Details",
			ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			// --- Inline prototype editing ---
			ImGui::PushID("PrototypeEditor");
			prototype->DrawImGui();
			ImGui::PopID();

			ImGui::Dummy(ImVec2(0, 4));

			// --- Action Buttons ---
			if (ImGui::Button("Regenerate Children", ImVec2(-FLT_MIN, 0))) {
				RegenerateChildren();
			}

			if (ImGui::Button("Remove Prototype", ImVec2(-FLT_MIN, 0))) {
				prototype = nullptr;
				children.clear();
			}

			ImGui::TreePop();
		}
	}
}

void UIRepeatContainer::SetPrototype(std::unique_ptr<UIElement> element) {
	prototype = std::move(element);
}

void UIRepeatContainer::RegenerateChildren() {
	children.clear();
	if (!prototype) return;
	if (count <= 0) return;

	// Basic layout: if rows <= 1 -> single row
	int rowsToUse = (rows <= 1) ? 1 : rows;
	int cols = (rowsToUse == 1) ? count : ((count + rowsToUse - 1) / rowsToUse);

	// Use spacing.x for horizontal, spacing.y for vertical
	for (int idx = 0; idx < count; ++idx) {
		//Each item is its own element, so it needs its own ids
		auto item = DelusiveInstantiate<UIElement>(*prototype.get(), instance);
		if (!item) continue;

		int r = (rowsToUse == 1) ? 0 : (idx / cols);   // row index
		int c = (rowsToUse == 1) ? idx : (idx % cols); // column index

		glm::vec2 offset = glm::vec2(spacing * float(c), spacing * float(r)); //TODO: change spacing to a vec2 for x and y spacing
		item->Move(position + offset - item->GetPosition());

		// AddChild should take ownership and set parent properly
		AddChild(std::move(item));
	}
}