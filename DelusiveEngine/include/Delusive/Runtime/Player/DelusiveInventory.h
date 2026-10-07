#pragma once
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <Delusive/Runtime/Core/UUID.h>
#include <memory>
#include <vector>
#include <string>

class Talisman;
class GameManager;
struct DelusiveInstance;

//The player's talismans. Saved as an [Inventory] block listing its talismans by UUID -
//equipped slots keep their positions, empty ones included - plus one block per talisman.
class DelusiveInventory {
public:
    DelusiveInventory();
    ~DelusiveInventory();
    DelusiveInventory(const DelusiveInventory&) = delete;
    DelusiveInventory& operator=(const DelusiveInventory&) = delete;
    DelusiveInventory(DelusiveInventory&&) noexcept;
    DelusiveInventory& operator=(DelusiveInventory&&) noexcept;

    void Link(GameManager* link) { gameManagerLink = link; }

    //Keeps whatever still fits when shrinking
    void SetSlotCount(int count);
	int GetSlotCount() const { return slotCount; }

    std::vector<Talisman*> GetAvailableTalismans();
	std::vector<Talisman*> GetEquippedTalismans();

    //Handle talisman equipping
    void EquipTalisman(int, std::unique_ptr<Talisman>);
    void EquipTalisman(int, Talisman*);
    void UnequipTalisman(int);
    //Type names come from DelusiveFactory<Talisman>, e.g. "BasicTalisman"
    void AddTalisman(const std::string&);

    //Identity and block entry points
    UUID GetID() const { return id; }
    void SetID(UUID newID) { id = newID; }
    void Serialize(DelusiveParser::DataBlock& out) const;
    void Deserialize(DelusiveParser::DataBlock& in);
    //Emits the inventory then every talisman it holds, flat
    void CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const;

    //Through the library like every other file - loading re-reads it from disk
    bool SaveToFile(const std::string& path);
    bool LoadFromFile(const std::string& path);

private:
    DelusiveInstance* Instance() const;

    GameManager* gameManagerLink = nullptr;
    UUID id;

    int slotCount = 5;
    std::vector<std::unique_ptr<Talisman>> availableTalismans;
    //Always slotCount long - a null entry is an empty slot
    std::vector<std::unique_ptr<Talisman>> equippedTalismans;
};
