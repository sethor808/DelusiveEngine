#include <DelusiveExternal/DelusiveInventoryData.h>
#include <DelusiveExternal/DelusiveTalismanData.h>
#include <Delusive/Runtime/Player/DelusiveInventory.h>

int DelusiveInventoryData::GetSlotCount() const{
    return IsValid() ? inventoryLink->GetSlotCount() : -1;
}

std::vector<DelusiveTalismanData> DelusiveInventoryData::GetAvailableTalismans() {
    std::vector<DelusiveTalismanData> data;
    
    if (!IsValid()) {
        return data;
    }
    
    std::vector<Talisman*> talismans = inventoryLink->GetAvailableTalismans();

    for (Talisman* t : talismans) {
        DelusiveTalismanData talismanData;
        talismanData.Link(t);
        data.emplace_back(talismanData);
    }
    
    return data;
}

std::vector<DelusiveTalismanData> DelusiveInventoryData::GetEquippedTalismans() {
    std::vector<DelusiveTalismanData> data;

    if (!IsValid()) {
        return data;
    }

    std::vector<Talisman*> talismans = inventoryLink->GetEquippedTalismans();

    for (Talisman* t : talismans) {
        DelusiveTalismanData talismanData;
        talismanData.Link(t);
        data.emplace_back(talismanData);
    }

    return data;
}

void DelusiveInventoryData::EquipTalisman(int slot, DelusiveTalismanData toEquip) {
    if (!IsValid()) return;

    inventoryLink->EquipTalisman(slot, toEquip.GetLink());
}

void DelusiveInventoryData::UnequipTalisman(int slot) {
    if (!IsValid()) return;

    inventoryLink->UnequipTalisman(slot);
}

void DelusiveInventoryData::AddTalisman(std::string& toAddType) {
    inventoryLink->AddTalisman(toAddType);
}