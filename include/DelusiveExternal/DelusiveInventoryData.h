//Script Facing Bridge
#pragma once
#include <DelusiveExternal/DelusiveTalismanData.h>
#include <vector>
#include <string>

class DelusiveInventory;

class DelusiveInventoryData {
public:
    //Limited to getters until decided otherwise
    bool IsValid() const { return inventoryLink; }
    void Link(DelusiveInventory* inventoryLink) { this->inventoryLink = inventoryLink; }

    int GetSlotCount() const;
    std::vector<DelusiveTalismanData> GetAvailableTalismans();
    std::vector<DelusiveTalismanData> GetEquippedTalismans();

    void EquipTalisman(int, DelusiveTalismanData);
    void UnequipTalisman(int);
    void AddTalisman(std::string& toAddType);
private:
    DelusiveInventory* inventoryLink;
};