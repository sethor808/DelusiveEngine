#pragma once
#include <Delusive/Scripting/UIScript.h>
#include <Delusive/Runtime/Core/IDLink.h>
#include <Scripts/UI/TalismanVisual.h>

class Talisman;
class UIRepeatContainer;
class DelusiveInventory;
class PlayerAgent;

//Equipped slots in one container, available talismans in the other. Clicking an available
//talisman equips a copy; clicking a slot empties it and makes it the next one filled.
class EquipScreen : public UIScript {
public:
	EquipScreen();
	void RegisterProperties() override;

	std::string GetType() const override { return "EquipScreen"; }
	void RelocateReferences() override;
	void Link(UIScriptContainer*) override;

	void OnInit() override;
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
	//Rebuilding inside a click would clear the clicked button mid-call
	bool needsRebuild = true;
	bool closeRequested = false;

	bool ReadyCheck();
	void Rebuild();
	void EnsureTemplate(UIRepeatContainer*);
	void BuildAvailableList();
	void BuildEquippedSlots();
	void EquipToSlot(Talisman*);
	void LoadoutChanged();
};
