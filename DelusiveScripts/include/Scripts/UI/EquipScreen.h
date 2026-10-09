#pragma once
#include <Delusive/Scripting/UIScript.h>
#include <Delusive/Runtime/Core/IDLink.h>

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

    //Display settings
    float iconSize = 1.0f;
    float stringSpacing = 1.0f;
    float stringSize = 1.0f;
    float stringXOffset = 1.0f, stringYOffset = 1.0f;

    //Rebuilding inside a click would delete the clicked button
    bool needsRebuild = true;
    bool closeRequested = false;

    bool ReadyCheck();
    void Rebuild();
    void BuildTalismanVisual(UIElement*, Talisman*);
    void BuildAvailableList();
    void BuildEquippedSlots();
    void EquipToFirstOpenSlot(Talisman*);
};