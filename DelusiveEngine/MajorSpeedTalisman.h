#pragma once
#include "Talisman.h"

class MajorSpeedTalisman : public Talisman {
	std::string GetType() override { return "MajorSpeed"; }
};