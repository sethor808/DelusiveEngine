#include "UIEquipScreen.h"
#include "DelusiveRenderer.h"
#include "DelusiveTalismans.h"

UIEquipScreen::UIEquipScreen(DelusiveRenderer& renderer) : UIElement(renderer) {
	name = "New UIEquipScreen";
}

std::unique_ptr<UIElement> UIEquipScreen::Clone() const {
	auto copy = std::make_unique<UIEquipScreen>(renderer);
	copy->SetPosition(position);
	copy->SetName(name);
	copy->SetEnabled(enabled);

	for (const auto& child : children) {
		copy->AddChild(std::move(child->Clone()));
	}

	return copy;
}

void UIEquipScreen::Draw(const glm::mat4& projection) {
	if (!enabled) return;



	//Draw highlight here maybe?

	UIElement::Draw(projection);
}