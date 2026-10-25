#pragma once
#include <Delusive/Scripting/UIScript.h>
#include <Delusive/Runtime/Core/IDLink.h>
#include <Scripts/UI/TalismanVisual.h>
#include <functional>
#include <string>
#include <vector>

class Talisman;
class UIRepeatContainer;
class DelusiveInventory;
class PlayerAgent;

//Equipped slots in one container, available talismans in the other. Clicking an available
//talisman equips a copy; clicking a slot empties it and makes it the next one filled.
//Talismans are drawn under this script's container, on top of the slots.
class EquipScreen : public UIScript {
public:
	EquipScreen();
	void RegisterProperties() override;

	std::string GetType() const override { return "EquipScreen"; }
	void RelocateReferences() override;
	void Link(UIScriptContainer*) override;

	void OnUpdate(float) override;
private:
	DelusiveUILink availableContainerID;
	DelusiveUILink equippedContainerID;
	//Optional - clicking it closes the screen
	DelusiveUILink confirmButtonID;
	TalismanVisualStyle style;

	UIRepeatContainer* availableContainer = nullptr;
	UIRepeatContainer* equippedContainer = nullptr;
	DelusiveInventory* inventoryData = nullptr;
	PlayerAgent* player = nullptr;

	//Slot the next pick goes into, -1 for the first empty one
	int targetSlot = -1;
	bool closeRequested = false;
	std::string builtFrom;

	bool ReadyCheck();
	void EnsureTemplate(UIRepeatContainer*);
	void Describe(std::string&, UIRepeatContainer*, const std::vector<Talisman*>&);
	void BuildSlots(UIRepeatContainer*, const std::vector<Talisman*>&, const std::function<void(int, Talisman*)>&);
	void EquipToSlot(Talisman*);
};
