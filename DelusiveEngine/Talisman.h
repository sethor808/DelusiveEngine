#pragma once
#include <string>

class PlayerAgent;

class Talisman {
public:
	Talisman() = default;

	virtual void Link(PlayerAgent* p) { player = p; }

	virtual std::string GetType() = 0;

	virtual int GetCurrentHP() { return hp; }
	virtual bool TakeDamage();

	virtual bool GetIsBroken() { return isBroken; }
	virtual void SetIsBroken(bool broken) { isBroken = broken; }

	virtual void GetConstantPassive() {}
	virtual void WhileActive() {}
	virtual void OnConsume() {}

	virtual void Reset() { hp = maxHP; isBroken = false; } //TODO: Write a graphical reset

protected:
	PlayerAgent* player; //shallow
	int maxHP = 2;
	int hp = maxHP;
	bool isBroken;
	std::string talismanBase;
	std::string talismanGlyph;
};