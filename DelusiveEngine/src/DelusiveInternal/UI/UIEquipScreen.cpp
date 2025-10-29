#include <DelusiveInternal/UI/UIEquipScreen.h>
#include <DelusiveInternal/Agents/PlayerAgent.h>
#include <DelusiveInternal/Rendering/DelusiveRenderer.h>
#include <DelusiveInternal/Talismans/DelusiveTalismans.h>
#include <DelusiveInternal/Player/DelusiveInventory.h>
#include <DelusiveInternal/UI/UIRepeatContainer.h>
#include <DelusiveInternal/UI/UITalismanButton.h>
#include <DelusiveInternal/UI/DelusiveUI.h>

UIEquipScreen::UIEquipScreen(DelusiveRenderer& renderer) : UIElement(renderer) {
	name = "New UIEquipScreen";
}

std::unique_ptr<UIElement> UIEquipScreen::Clone() const {
	auto copy = std::make_unique<UIEquipScreen>(renderer);

	copy->SetPosition(position);
	copy->SetSize(size);
	copy->SetName(name);
	copy->SetEnabled(enabled);

	return copy;
}

void UIEquipScreen::Init() {
	if (!player || !player->GetInventory()) return;

	availableTalismans = player->GetInventory()->GetAvailableTalismans();

	BuildEquipSlots();
	BuildAvailableTalismans();
}

void UIEquipScreen::BuildEquipSlots() {
	slotContainer = AddChild<UIRepeatContainer>();
	slotContainer->SetName("EquippedSlots");
	slotContainer->SetPosition({ position.x, position.y });
	slotContainer->SetSpacing(48.0f);

	auto slotProto = std::make_unique<UITalismanButton>(renderer);
	slotProto->SetSize({ 32, 32 });
	slotContainer->SetPrototype(std::move(slotProto));
	slotContainer->SetCount((int)equippedTalismans.size());

	// Hook buttons
	for (int i = 0; i < (int)equippedTalismans.size(); ++i) {
		auto* button = dynamic_cast<UITalismanButton*>(slotContainer->GetChildren()[i]);
		if (!button) continue;

		button->BindTalisman(equippedTalismans[i]);
		button->onClickTalisman = [this, i](Talisman* t) {
			selectedSlot = i;
			};
	}
}

void UIEquipScreen::BuildAvailableTalismans() {
	talismanContainer = AddChild<UIRepeatContainer>();
	talismanContainer->SetName("AvailableTalismans");
	talismanContainer->SetPosition({ position.x, position.y + 64.0f });
	talismanContainer->SetSpacing(48.0f);

	auto talismanProto = std::make_unique<UITalismanButton>(renderer);
	talismanProto->SetSize({ 32, 32 });
	talismanContainer->SetPrototype(std::move(talismanProto));
	talismanContainer->SetCount((int)availableTalismans.size());

	for (int i = 0; i < (int)availableTalismans.size(); ++i) {
		auto* button = dynamic_cast<UITalismanButton*>(slotContainer->GetChildren()[i]);
		if (!button) continue;

		button->BindTalisman(availableTalismans[i]);
		button->onClickTalisman = [this, i](Talisman* t) {
			if (selectedSlot >= 0 && t)
				Equip(selectedSlot, t);
			};
	}
}

void UIEquipScreen::Draw(const glm::mat4& projection) {
	if (!enabled) return;



	//Draw highlight here maybe?

	UIElement::Draw(projection);
}

void UIEquipScreen::Equip(int index, Talisman* talisman) {
	if (!player || index < 0 || index >= (int)equippedTalismans.size()) {
		return;
	}

	player->EquipTalisman(index, talisman);
	equippedTalismans[index] = talisman;
}