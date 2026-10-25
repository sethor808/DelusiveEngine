#pragma once
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <Delusive/Runtime/Core/UUID.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <memory>
#include <string>
#include <vector>

class PlayerAgent;
class DelusiveInventory;
class PropertyRegistry;
struct PlayerStats;
struct HitData;

//Built through DelusiveFactory<Talisman>, so it loads, saves and clones like everything else.
//The type sets maxHP and textures; a talisman's own state (hp, broken) is what gets saved.
class Talisman {
public:
	Talisman();
	virtual ~Talisman();
	//The registry points at this object's members
	Talisman(const Talisman&) = delete;
	Talisman& operator=(const Talisman&) = delete;

    virtual void Link(DelusiveInventory* link) { inventoryLink = link; }

	//Doubles as the factory key
	virtual const char* GetType() const = 0;

	//Identity and block entry points
	UUID GetID() const { return id; }
	void SetID(UUID newID) { id = newID; }
	void Serialize(DelusiveParser::DataBlock& out) const;
	void Deserialize(DelusiveParser::DataBlock& in);
	void CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const;

	virtual int GetMaxHP() { return maxHP; }
	virtual int GetCurrentHP() { return hp; }
	virtual bool TakeDamage(int);

	virtual bool GetIsBroken() { return isBroken; }
	virtual void SetIsBroken(bool broken) { isBroken = broken; }

	//Stat hooks: Passive while unbroken, WhileActive only on the top unbroken talisman, WhileBroken after it breaks
	virtual void ApplyPassive(PlayerStats&) const {}
	virtual void ApplyWhileActive(PlayerStats&) const {}
	virtual void ApplyWhileBroken(PlayerStats&) const {}
	//Fires once, the moment the talisman breaks
	virtual void OnConsume(PlayerAgent&) {}
	//Fire on every unbroken talisman; while-active effects check IsActive first
	virtual void HitEvent(PlayerAgent&, const HitData&) {}
	virtual void LandHitEvent(PlayerAgent&, const HitData&) {}

    virtual std::string ConstantPassiveDesc() { return ""; }
    virtual std::string WhileActiveDesc() { return ""; }
    virtual std::string OnConsumeDesc() { return ""; }
	virtual std::string WhileBrokenDesc() { return ""; }

	bool IsActive(const PlayerAgent&) const;

	virtual std::string GetBaseTexture() const;
	virtual std::string GetGlyphTexture() const;
	virtual std::string GetStringTexture() const;

	virtual void Reset() { hp = maxHP; isBroken = false; } //TODO: Write a graphical reset

protected:
    DelusiveInventory* inventoryLink = nullptr;
	UUID id;
	int maxHP = 2;
	int hp = maxHP;
	bool isBroken = false;
	std::unique_ptr<PropertyRegistry> registry;
	std::string talismanBase = DEFAULT_TALISMAN;
	//Empty for a talisman without a glyph
	std::string talismanGlyph;
	std::string talismanString = TALISMAN_STRING;
};