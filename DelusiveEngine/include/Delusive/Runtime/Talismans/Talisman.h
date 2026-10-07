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
	virtual std::string GetType() const = 0;

	//Identity and block entry points
	UUID GetID() const { return id; }
	void SetID(UUID newID) { id = newID; }
	void Serialize(DelusiveParser::DataBlock& out) const;
	void Deserialize(DelusiveParser::DataBlock& in);
	void CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const;

	virtual int GetMaxHP() { return maxHP; }
	virtual int GetCurrentHP() { return hp; }
	virtual bool TakeDamage();

	virtual bool GetIsBroken() { return isBroken; }
	virtual void SetIsBroken(bool broken) { isBroken = broken; }

	virtual void ConstantPassive() {}
	virtual void WhileActive() {}    
	virtual void OnConsume() {}

    virtual std::string ConstantPassiveDesc() { return ""; }
    virtual std::string WhileActiveDesc() { return ""; }
    virtual std::string OnConsumeDesc() { return ""; }

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
	std::string talismanGlyph = TALISMAN_STRING;
	std::string talismanString = TALISMAN_STRING;
};