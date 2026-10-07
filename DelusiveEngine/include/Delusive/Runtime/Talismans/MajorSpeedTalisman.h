#pragma once
#include <Delusive/Runtime/Talismans/Talisman.h>

class MajorSpeedTalisman : public Talisman {
public:
	std::string GetType() const override { return "MajorSpeedTalisman"; }
};