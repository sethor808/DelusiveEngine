#pragma once
#include <DelusiveExternal/UIScript.h>
#include <DelusiveExternal/UUID.h>
#include <DelusiveExternal/DelusiveInventoryData.h>
#include <DelusiveExternal/DelusiveUIRepeatContainer.h>
#include <optional>

class DelusiveTalismanData;
class DelusiveUIRepeatContainer;

class EquipScreen : public UIScript {
public:
    std::string GetType() const override { return "EquipScreen"; }

    void OnInit() override;
    void OnUpdate(float) override {};
    void OnDraw() override {};
    void OnEvent() override {};
    void OnClick(DelusiveUIElement* clicked) override {};

private:
    UUID availableContainerID;
    DelusiveUIRepeatContainer availableContainer;
    UUID equippedContainerID;
    DelusiveUIRepeatContainer equippedContainer;
    DelusiveInventoryData inventoryData;

    std::vector<DelusiveTalismanData> inventoryTalismans;
    std::vector<std::optional<DelusiveTalismanData>> equippedSlots;

    bool ReadyCheck();
    void BuildAvailableList();
    void BuildEquippedSlots();
    void EquipToFirstOpenSlot(const DelusiveTalismanData&);
};