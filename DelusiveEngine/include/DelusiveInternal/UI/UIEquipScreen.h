#pragma once
#include <DelusiveInternal/UI/UIElement.h>

class PlayerAgent;
class Talisman;
class UIButton;
class UIRepeatContainer;

class UIEquipScreen : public UIElement {
public:
	UIEquipScreen(DelusiveRenderer&);

	std::unique_ptr<UIElement> Clone() const override;
	const std::string GetType() const override { return "s"; }

	void Draw(const glm::mat4&) override;

	void Init();
	void BuildEquipSlots();
	void BuildAvailableTalismans();

	void LinkPlayer(PlayerAgent* p) { player = p; }

	void Equip(int, Talisman*);

private:
	PlayerAgent* player = nullptr;
	int selectedSlot = -1;

	std::vector<Talisman*> availableTalismans;
	std::vector<Talisman*> equippedTalismans;

	UIRepeatContainer* slotContainer = nullptr;
	UIRepeatContainer* talismanContainer = nullptr;
};