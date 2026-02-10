#include <Delusive/Runtime/Player/DelusiveInventory.h>
#include <Delusive/Runtime/Talismans/Talisman.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Runtime/Talismans/DelusiveTalismans.h>
#include <unordered_map>

DelusiveInventory::DelusiveInventory() {
    RegisterTalismans();
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
		if (!t) continue;
		result.push_back(t.get());
	}
	return result;
}

void DelusiveInventory::EquipTalisman(int index, std::unique_ptr<Talisman> talisman) {
	if (index < 0 || index >= slotCount) return;
	equippedTalismans[index] = std::move(talisman);
}

void DelusiveInventory::EquipTalisman(int slot, Talisman* t) {
    auto it = std::find_if(
        availableTalismans.begin(),
        availableTalismans.end(),
        [&](const std::unique_ptr<Talisman>& ptr) {
            return ptr.get() == t;
        }
    );

    if (it == availableTalismans.end()) return;

    //Exit out early, unsure if I want to remove talismans from available ones
    return;

    equippedTalismans[slot] = std::move(*it);
    availableTalismans.erase(it);
}

void DelusiveInventory::UnequipTalisman(int index) {
	if (index < 0 || index >= slotCount) return;
	equippedTalismans[index].reset();
}

void DelusiveInventory::AddTalisman(const std::string& type) {
    auto talisman = factory.Create(type);
    if (!talisman) return;

    talisman->Link(this);
    availableTalismans.push_back(std::move(talisman));
}

// --- Persistence ---
void DelusiveInventory::Serialize(std::ostream& out) const {
    out << slotCount << '\n';
    out << availableTalismans.size() << '\n';
    for (const auto& t : availableTalismans)
        out << t->GetType() << '\n';

    out << equippedTalismans.size() << '\n';
    for (const auto& t : equippedTalismans) {
        if (t)
            out << t->GetType() << '\n';
        else
            out << "NONE\n";
    }
}

void DelusiveInventory::Deserialize(std::istream& in, const std::unordered_map<std::string, std::function<std::unique_ptr<Talisman>()>>& factory) {
    in >> slotCount;

    size_t invCount = 0;
    in >> invCount;
    availableTalismans.clear();
    for (size_t i = 0; i < invCount; ++i) {
        std::string id;
        in >> id;
        if (auto it = factory.find(id); it != factory.end())
            availableTalismans.push_back(it->second());
    }

    size_t eqCount = 0;
    in >> eqCount;
    equippedTalismans.clear();
    equippedTalismans.resize(eqCount);
    for (size_t i = 0; i < eqCount; ++i) {
        std::string id;
        in >> id;
        if (id != "NONE") {
            if (auto it = factory.find(id); it != factory.end())
                equippedTalismans[i] = it->second();
        }
    }
}

//Presently this needs to be manually updated
void DelusiveInventory::RegisterTalismans() {
    factory.Register("BasicTalisman", []() {
        return std::make_unique<BasicTalisman>();
        });
}