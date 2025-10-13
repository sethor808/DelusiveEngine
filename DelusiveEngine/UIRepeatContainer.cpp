#include "UIRepeatContainer.h"
#include "DelusiveRegistry.h"

UIRepeatContainer::UIRepeatContainer(DelusiveRenderer& renderer) 
	: UIElement(renderer), prototype(nullptr)
{
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
		copy->prototype = prototype->Clone();
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

void UIRepeatContainer::DrawImGui() {
	UIElement::DrawImGui();
	// --- Prototype editing section ---
	if (ImGui::CollapsingHeader("Prototype", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (!prototype) {
			ImGui::TextDisabled("No prototype assigned.");
		}
		else {
			ImGui::PushID("PrototypeEditor");
			prototype->DrawImGui(); // inline prototype editing
			ImGui::PopID();
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
		auto item = prototype->Clone();
		if (!item) continue;

		int r = (rowsToUse == 1) ? 0 : (idx / cols);   // row index
		int c = (rowsToUse == 1) ? idx : (idx % cols); // column index

		glm::vec2 offset = glm::vec2(spacing * float(c), spacing * float(r)); //TODO: change spacing to a vec2 for x and y spacing
		item->SetPosition(position + offset);

		// AddChild should take ownership and set parent properly
		AddChild(std::move(item));
	}
}