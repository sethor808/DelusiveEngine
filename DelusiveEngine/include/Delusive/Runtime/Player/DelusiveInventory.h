#pragma once
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <unordered_map>
#include <Delusive/Runtime/Talismans/TalismanFactory.h>

class Talisman;
class GameManager;

class DelusiveInventory {
public:
    DelusiveInventory();
    DelusiveInventory(const DelusiveInventory&) = delete;
    DelusiveInventory& operator=(const DelusiveInventory&) = delete;
    DelusiveInventory(DelusiveInventory&&) noexcept = default;
    DelusiveInventory& operator=(DelusiveInventory&&) noexcept = default;

    void RegisterTalismans();
    void Link(GameManager* link) { gameManagerLink = link; }

    void SetSlotCount(int count) {slotCount = count;}
	int GetSlotCount() const { return slotCount; }

    std::vector<Talisman*> GetAvailableTalismans();
	std::vector<Talisman*> GetEquippedTalismans();

    //Handle talisman equipping
    void EquipTalisman(int, std::unique_ptr<Talisman>);
    void EquipTalisman(int, Talisman*);
    void UnequipTalisman(int);
    void AddTalisman(const std::string&);

    void Serialize(std::ostream&) const;
    void Deserialize(std::istream&, const std::unordered_map<std::string, std::function<std::unique_ptr<Talisman>()>>&);
private:
    GameManager* gameManagerLink;
    TalismanFactory factory;

    int slotCount = 5;
    std::vector<std::unique_ptr<Talisman>> availableTalismans;
    std::vector<std::unique_ptr<Talisman>> equippedTalismans;
};