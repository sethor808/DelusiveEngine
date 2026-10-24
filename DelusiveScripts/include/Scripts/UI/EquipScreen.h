#pragma once
#include <Delusive/Scripting/UIScript.h>
#include <Delusive/Runtime/Core/IDLink.h>
#include <Scripts/UI/TalismanVisual.h>

class Talisman;
class UIElement;
class UIRepeatContainer;
class DelusiveInventory;
class PlayerAgent;

class EquipScreen : public UIScript {
public:
    EquipScreen();
    void RegisterProperties() override;

    void RelocateReferences() override;
    std::string GetType() const override { return "EquipScreen"; }
    virtual void Link(UIScriptContainer*) override;

    void OnInit() override;
	void OnUpdate(float) override;
    void OnDraw() override {};
    void OnEvent() override {};
    void OnClick(UIScriptContainer* clicked) override {}; //idk what this is supposed to be

private:
    DelusiveUILink availableContainerID;
    UIRepeatContainer* availableContainer = nullptr;
    DelusiveUILink equippedContainerID;
    UIRepeatContainer* equippedContainer = nullptr;
	//Optional - clicking it hides the screen
	DelusiveUILink confirmButtonID;
    DelusiveInventory* inventoryData = nullptr;
	PlayerAgent* player = nullptr;

	TalismanVisualStyle style;

	//Rebuilding inside a click would delete the clicked button
	bool needsRebuild = true;
	bool closeRequested = false;

    bool ReadyCheck();
	void Rebuild();
    void BuildAvailableList();
    void BuildEquippedSlots();
    void EquipToFirstOpenSlot(Talisman*);
};