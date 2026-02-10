#pragma once
#include <DelusiveInternal/Talismans/Talisman.h>

class SpeedTalisman : public Talisman {
	std::string GetType() override { return "Speed"; }
};