#pragma once
#include "Talisman.h"

class SpeedTalisman : public Talisman {
	std::string GetType() override { return "Speed"; }
};