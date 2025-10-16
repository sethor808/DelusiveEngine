#define DELUSIVE_EDITOR_MODE
#include "UIRepeatContainer.h"
#include "DelusiveRegistry.h"
#include "DelusiveRenderer.h"
#include "DelusiveUI.h"
#include "DelusiveData.h"

UIRepeatContainer::UIRepeatContainer(DelusiveRenderer& renderer)
	: UIElement(renderer)
{
	prototype = std::make_unique<DelusiveUIPrototype>();
	prototype->element = nullptr;
	name = "New UIRepeatContainer";
	RegisterProperties();
}

std::unique_ptr<UIElement> UIRepeatContainer::Clone() const {
	auto copy = std::make_unique<UIRepeatContainer>(renderer);

	copy->SetPosition(position);
	copy->SetSize(size);
	copy->count = count;
	copy->rows = rows;
	copy->spacing = spacing;

	if (prototype) {
		// prototype->Clone() returns unique_ptr<UIElement>
		copy->prototype->element = prototype->element->Clone();
	}

	// Caller can call RegenerateChildren() when appropriate.
	return copy;
}

void UIRepeatContainer::RegisterProperties() {
	UIElement::RegisterProperties();
	registry->Register("count", &count);
	registry->Register("rows", &rows);
	registry->Register("spacing", &spacing);
}

void UIRepeatContainer::Draw(const glm::mat4& projection) {
	UIElement::Draw(projection);

	for(auto& child : children){
		if (child) {
			child->Draw(projection);
		}
	}

#ifdef DELUSIVE_EDITOR_MODE
	if (prototype && prototype->element) {
		renderer.PushAlpha(0.3f);
		prototype->element->Draw(projection);
		renderer.PopAlpha();
	}
#endif
}

void UIRepeatContainer::DrawImGui() {
	UIElement::DrawImGui();
	// --- Prototype editing section ---
	// --- Prototype Management Section ---
	ImGui::SeparatorText("Prototype");

	if (!prototype->element) {
		ImGui::TextDisabled("No prototype assigned.");

		if (ImGui::Button("Create Prototype", ImVec2(-FLT_MIN, 0))) {
			ImGui::OpenPopup("AddPrototypePopup");
		}

		if (ImGui::BeginPopup("AddPrototypePopup")) {
			std::string type = DelusiveUI::DrawUIElementAddMenu();
			if (!type.empty()) {
				auto newProto = DelusiveUI::CreateUIElementByType(type, renderer);
				if (newProto) {
					newProto->LinkCanvas(parentCanvas);
					prototype->element = std::move(newProto);
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
			prototype->element->DrawImGui();
			ImGui::PopID();

			ImGui::Dummy(ImVec2(0, 4));

			// --- Action Buttons ---
			if (ImGui::Button("Regenerate Children", ImVec2(-FLT_MIN, 0))) {
				RegenerateChildren();
			}

			if (ImGui::Button("Remove Prototype", ImVec2(-FLT_MIN, 0))) {
				prototype.reset();
				children.clear();
			}

			ImGui::TreePop();
		}
	}
}

void UIRepeatContainer::SetPrototype(std::unique_ptr<UIElement> element) {
	prototype->element = std::move(element);
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
		auto item = prototype->element->Clone();
		if (!item) continue;

		int r = (rowsToUse == 1) ? 0 : (idx / cols);   // row index
		int c = (rowsToUse == 1) ? idx : (idx % cols); // column index

		glm::vec2 offset = glm::vec2(spacing * float(c), spacing * float(r)); //TODO: change spacing to a vec2 for x and y spacing
		item->SetPosition(position + offset);

		// AddChild should take ownership and set parent properly
		AddChild(std::move(item));
	}
}