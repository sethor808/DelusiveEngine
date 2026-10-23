#pragma once
#include <Delusive/Runtime/Talismans/Talisman.h>
#include <Delusive/Runtime/Player/PlayerStats.h>

class MajorSpeedTalisman : public Talisman {
public:
	const char* GetType() const override { return "MajorSpeedTalisman"; }

	void ApplyWhileActive(PlayerStats&) const override;
	std::string WhileActiveDesc() override { return "+25% move and dodge speed"; }
};
