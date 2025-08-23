#pragma once

class Talisman {
public:
	Talisman() = default;

	virtual int GetCurrentHP() { return hp; }
	virtual bool TakeDamage();

	virtual bool GetIsBroken() { return isBroken; }
	virtual void SetIsBroken(bool broken) { isBroken = broken; }

	virtual void GetConstantPassive() {}
	virtual void WhileActive() {}
	virtual void OnConsume() {}

protected:
	int hp = 2;
	bool isBroken;
};