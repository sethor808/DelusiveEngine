#include <Delusive/Runtime/Player/DelusiveInventory.h>
#include <Delusive/Runtime/Talismans/Talisman.h>
#include <Delusive/Runtime/Core/DelusiveClone.h>
#include <Delusive/Runtime/Core/DelusiveLibrary.h>
#include <Delusive/Runtime/Core/GameManager.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

DelusiveInventory::DelusiveInventory() {
    equippedTalismans.resize(slotCount);
}

DelusiveInventory::~DelusiveInventory() = default;
DelusiveInventory::DelusiveInventory(DelusiveInventory&&) noexcept = default;
DelusiveInventory& DelusiveInventory::operator=(DelusiveInventory&&) noexcept = default;

DelusiveInstance* DelusiveInventory::Instance() const {
    return gameManagerLink ? &gameManagerLink->GetInstance() : nullptr;
}

void DelusiveInventory::SetSlotCount(int count) {
    slotCount = std::max(count, 0);
    equippedTalismans.resize(slotCount);
}

std::vector<Talisman*> DelusiveInventory::GetAvailableTalismans() {
	std::vector<Talisman*> result;
	result.reserve(availableTalismans.size());

	for (auto& t : availableTalismans) {
		result.push_back(t.get());
	}
	return result;
}

std::vector<Talisman*> DelusiveInventory::GetEquippedTalismans() {
	std::vector<Talisman*> result;
	result.reserve(equippedTalismans.size());

	for (auto& t : equippedTalismans) {
		result.push_back(t.get());
	}
	return result;
}

void DelusiveInventory::EquipTalisman(int index, std::unique_ptr<Talisman> talisman) {
	if (index < 0 || index >= slotCount) return;
	if (talisman) {
		//Needs an identity to be saved in its slot
		if (!talisman->GetID().IsValid()) talisman->SetID(UUID::GenerateRandom());
		talisman->Link(this);
	}
	equippedTalismans[index] = std::move(talisman);
}

void DelusiveInventory::EquipTalisman(int slot, Talisman* t) {
	if (slot < 0 || slot >= slotCount) return;

    auto it = std::find_if(
        availableTalismans.begin(),
        availableTalismans.end(),
        [&](const std::unique_ptr<Talisman>& ptr) {
            return ptr.get() == t;
        }
    );
    if (it == availableTalismans.end()) return;

	std::unique_ptr<Talisman> talisman = std::move(*it);
    availableTalismans.erase(it);
	UnequipTalisman(slot);
	equippedTalismans[slot] = std::move(talisman);
}

void DelusiveInventory::UnequipTalisman(int index) {
	if (index < 0 || index >= slotCount) return;
	if (equippedTalismans[index]) availableTalismans.push_back(std::move(equippedTalismans[index]));
}

void DelusiveInventory::AddTalisman(const std::string& type) {
    DelusiveInstance* instance = Instance();
    if (!instance) {
        std::cerr << "[Inventory] Not linked to a GameManager - cannot create " << type << std::endl;
        return;
    }

    auto talisman = DelusiveFactory<Talisman>::Create(type, *instance);
    if (!talisman) {
        std::cerr << "[Inventory] Unknown talisman type: " << type << std::endl;
        return;
    }

    talisman->SetID(UUID::GenerateRandom());
    talisman->Link(this);
    availableTalismans.push_back(std::move(talisman));
}

#pragma region Serialization

void DelusiveInventory::Serialize(DelusiveParser::DataBlock& out) const {
    out.category = "Inventory";
    out.properties["id"] = id.ToString();
    out.properties["slotCount"] = std::to_string(slotCount);

    //Ownership lists - each talisman writes its own block
    std::ostringstream available;
    for (const auto& t : availableTalismans) {
        if (t) available << t->GetID().ToString() << " ";
    }
    out.properties["available"] = available.str();

    //One entry per slot so positions survive; "" is an empty slot
    std::ostringstream equipped;
    equipped << equippedTalismans.size();
    for (const auto& t : equippedTalismans) {
        equipped << " " << std::quoted(t ? t->GetID().ToString() : std::string());
    }
    out.properties["equipped"] = equipped.str();
}

void DelusiveInventory::Deserialize(DelusiveParser::DataBlock& in) {
    DelusiveInstance* instance = Instance();
    if (!instance) {
        std::cerr << "[Inventory] Not linked to a GameManager - cannot load" << std::endl;
        return;
    }

    auto Build = [this, instance](const std::string& idText) -> std::unique_ptr<Talisman> {
        UUID talismanID;
        talismanID.FromString(idText);

        const DelusiveParser::DataBlock* recipe = instance->delusiveLibrary.Find(talismanID);
        if (!recipe) {
            std::cerr << "[Inventory] Missing talisman " << idText << std::endl;
            return nullptr;
        }

        auto talisman = DelusiveBuild<Talisman>(*recipe, *instance);
        if (talisman) talisman->Link(this);
        return talisman;
    };

    if (auto prop = in.properties.find("id"); prop != in.properties.end()) id.FromString(prop->second);

    int slots = slotCount;
    if (auto prop = in.properties.find("slotCount"); prop != in.properties.end()) {
        std::istringstream(prop->second) >> slots;
    }

    availableTalismans.clear();
    if (auto prop = in.properties.find("available"); prop != in.properties.end()) {
        std::istringstream ids(prop->second);
        std::string idText;
        while (ids >> idText) {
            if (auto talisman = Build(idText)) availableTalismans.push_back(std::move(talisman));
        }
    }

    equippedTalismans.clear();
    SetSlotCount(slots);
    if (auto prop = in.properties.find("equipped"); prop != in.properties.end()) {
        std::istringstream list(prop->second);
        size_t count = 0;
        list >> count;
        for (size_t slot = 0; slot < count; ++slot) {
            std::string idText;
            if (!(list >> std::quoted(idText))) break;
            if (idText.empty() || slot >= equippedTalismans.size()) continue;
            equippedTalismans[slot] = Build(idText);
        }
    }
}

void DelusiveInventory::CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const {
    DelusiveParser::DataBlock self;
    Serialize(self);
    self.id = id;
    out.push_back(std::move(self));

    for (const auto& t : availableTalismans) if (t) t->CollectBlocks(out);
    for (const auto& t : equippedTalismans)  if (t) t->CollectBlocks(out);
}

bool DelusiveInventory::SaveToFile(const std::string& path) {
    DelusiveInstance* instance = Instance();
    if (!instance) return false;
    if (!id.IsValid()) id = UUID::GenerateRandom();

    std::vector<DelusiveParser::DataBlock> blocks;
    CollectBlocks(blocks);
    return instance->delusiveLibrary.WriteFile(path, std::move(blocks));
}

bool DelusiveInventory::LoadFromFile(const std::string& path) {
    DelusiveInstance* instance = Instance();
    if (!instance || !instance->delusiveLibrary.LoadFile(path)) return false;

    for (const DelusiveParser::DataBlock* block : instance->delusiveLibrary.ListFile(path)) {
        if (block->category == "Inventory") {
            Deserialize(const_cast<DelusiveParser::DataBlock&>(*block));
            id = block->id;
            return true;
        }
    }

    std::cerr << "[Inventory] No Inventory block in " << path << std::endl;
    return false;
}

#pragma endregion
