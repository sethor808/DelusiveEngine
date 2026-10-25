#include <Delusive/Runtime/UI/UIRepeatContainer.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Core/DelusiveClone.h>
#include <imgui/imgui.h>

UIRepeatContainer::UIRepeatContainer(DelusiveInstance& instance)
	: UIElement(instance)
{
	name = "New UIRepeatContainer";
	RegisterProperties();
}

void UIRepeatContainer::RegisterProperties() {
	UIElement::RegisterProperties();
	registry->Register("rows", &rows);
	registry->Register("spacing", &spacing);
}

void UIRepeatContainer::Update(float deltaTime) {
	//Children sit on a grid from the container's position, so moving the container moves them
	const int count = static_cast<int>(children.size());
	const int cols = (count + rows - 1) / rows;

	for (int i = 0; i < count; ++i) {
		const int row = rows == 1 ? 0 : i / cols;
		const int col = rows == 1 ? i : i % cols;

		//TODO: change spacing to a vec2 for x and y spacing
		const glm::vec2 target = position + glm::vec2(spacing * static_cast<float>(col), spacing * static_cast<float>(row));
		children[i]->Move(target - children[i]->GetPosition());
	}

	UIElement::Update(deltaTime);
}

void UIRepeatContainer::SetCount(int count) {
	if (children.empty()) return;

	//Each copy is its own element, so it needs its own ids
	while (static_cast<int>(children.size()) < count) {
		auto copy = DelusiveInstantiate<UIElement>(*children.front(), instance);
		if (!copy) return;
		AddChild(std::move(copy));
	}

	//Extras are hidden rather than dropped so the template survives a count of 0
	for (int i = 1; i < static_cast<int>(children.size()); ++i) {
		children[i]->SetEnabled(i < count);
	}
}

void UIRepeatContainer::DrawChildrenImGui() {
	UIElement::DrawChildrenImGui();
	ImGui::TextDisabled("The first child is the template, copied for each item");
}
