#pragma once
#include "UIElement.h"

class Talisman;
class UIButton;

class UIEquipScreen : public UIElement {
public:
	UIEquipScreen(DelusiveRenderer&);

	std::unique_ptr<UIElement> Clone() const override;
	const std::string GetType() const override { return "UIEquipScreen"; }

	void Draw(const glm::mat4&) override;

private:
	int selectedSlot = -1;
	int selectedTalisman = -1;
	std::vector<std::shared_ptr<Talisman>> availableTalismans;
	std::vector<std::shared_ptr<Talisman>> equippedTalismans;
	std::vector<std::unique_ptr<UIButton>> slotButtons;
	std::vector<std::unique_ptr<UIButton>> talismanButtons;
};