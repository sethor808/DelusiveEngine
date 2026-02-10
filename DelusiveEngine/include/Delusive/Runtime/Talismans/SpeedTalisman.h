#pragma once
#include <Delusive/Runtime/Talismans/Talisman.h>

class SpeedTalisman : public Talisman {
	std::string GetType() override { return "Speed"; }
};