#pragma once
#include <string>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>

class PlayerAgent;
class InventoryLink;
class DelusiveInventory;

class Talisman {
public:
	Talisman() = default;

    virtual void Link(DelusiveInventory* link) { inventoryLink = link; }

	virtual std::string GetType() = 0;

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
    DelusiveInventory* inventoryLink;
	int maxHP = 2;
	int hp = maxHP;
	bool isBroken;
	std::string talismanBase = DEFAULT_TALISMAN;
	std::string talismanGlyph = TALISMAN_STRING;
	std::string talismanString = TALISMAN_STRING;
};