#pragma once
#include <Delusive/Runtime/Talismans/Talisman.h>
#include <Delusive/Runtime/Player/PlayerStats.h>

class SpeedTalisman : public Talisman {
public:
	const char* GetType() const override { return "SpeedTalisman"; }

	void ApplyPassive(PlayerStats& stats) const override { stats.moveSpeed *= 1.1f; }
	std::string ConstantPassiveDesc() override { return "+10% move speed"; }
};
