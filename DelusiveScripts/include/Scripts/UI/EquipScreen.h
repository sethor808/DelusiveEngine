#pragma once
#include <Delusive/Scripting/UIScript.h>
#include <Delusive/Runtime/Utils/UUID.h>
#include <Delusive/Runtime/Talismans/Talisman.h>
#include <optional>

class UIElement;
class UIRepeatContainer;
class DelusiveInventory;

class EquipScreen : public UIScript {
public:
    std::string GetType() const override { return "EquipScreen"; }

    void OnInit() override;
    void OnUpdate(float) override {};
    void OnDraw() override {};
    void OnEvent() override {};
    void OnClick(UIScriptContainer* clicked) override {}; //idk what this is supposed to be

private:
    UUID availableContainerID;
    UIRepeatContainer* availableContainer = nullptr;
    UUID equippedContainerID;
    UIRepeatContainer* equippedContainer = nullptr;
    DelusiveInventory* inventoryData = nullptr;

    //Display settings - make visible in editor
    float iconSize = 1.0f;
    float stringSpacing = 1.0f;
    float stringSize = 1.0f;
    float stringXOffset = 1.0f, stringYOffset = 1.0f;

    std::vector<Talisman*> inventoryTalismans;
    std::vector<std::optional<Talisman*>> equippedSlots;

    bool ReadyCheck();
    void BuildTalismanVisual(UIElement*, Talisman*);
    void BuildAvailableList();
    void BuildEquippedSlots();
    void EquipToFirstOpenSlot(Talisman*);
};