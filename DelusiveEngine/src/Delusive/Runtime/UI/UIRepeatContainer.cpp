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
}

void UIRepeatContainer::Update(float deltaTime) {
	//Dragged by the inspector rather than moved by code
	if (position != placedAt) {
		const glm::vec2 delta = position - placedAt;
		placedAt = position;
		for (auto& child : children) {
			child->Move(delta);
		}
	}

	UIElement::Update(deltaTime);
}

void UIRepeatContainer::Move(const glm::vec2& delta) {
	UIElement::Move(delta);
	placedAt = position;
}

void UIRepeatContainer::Draw(const glm::mat4& projection) {
	if (!enabled) return;
	UIElement::Draw(projection);

#ifdef DELUSIVE_EDITOR_MODE
	//Shows where copies will go while there are none
	if (prototype && children.empty()) {
		instance.renderer.PushAlpha(0.3f);
		prototype->Draw(projection);
		instance.renderer.PopAlpha();
	}
#endif
}

void UIRepeatContainer::DrawChildrenImGui() {
	ImGui::SeparatorText("Prototype");

	if (!prototype) {
		ImGui::TextDisabled("No prototype assigned.");

		if (ImGui::Button("Create Prototype", ImVec2(-FLT_MIN, 0))) {
			ImGui::OpenPopup("AddPrototypePopup");
		}

		if (ImGui::BeginPopup("AddPrototypePopup")) {
			std::string type = DelusiveUI::DrawUIElementAddMenu();
			if (!type.empty()) {
				auto newPrototype = DelusiveUI::CreateUIElementByType(type, instance);
				if (newPrototype) {
					newPrototype->LinkCanvas(parentCanvas);
					prototype = std::move(newPrototype);
					RegenerateChildren();
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		return;
	}

	ImGui::PushID("PrototypeEditor");
	prototype->DrawImGui();
	ImGui::PopID();

	ImGui::Dummy(ImVec2(0, 4));

	if (ImGui::Button("Remove Prototype", ImVec2(-FLT_MIN, 0))) {
		prototype = nullptr;
		children.clear();
	}
}

void UIRepeatContainer::SetPrototype(std::unique_ptr<UIElement> element) {
	prototype = std::move(element);
}

void UIRepeatContainer::RegenerateChildren() {
	children.clear();
	placedAt = position;
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