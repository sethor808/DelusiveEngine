#pragma once
#include <DelusiveInternal/Talismans/Talisman.h>

class MajorSpeedTalisman : public Talisman {
	std::string GetType() override { return "MajorSpeed"; }
};