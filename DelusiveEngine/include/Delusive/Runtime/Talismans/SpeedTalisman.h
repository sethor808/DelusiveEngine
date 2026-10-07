#pragma once
#include <Delusive/Runtime/Talismans/Talisman.h>

class SpeedTalisman : public Talisman {
public:
	std::string GetType() const override { return "SpeedTalisman"; }
};