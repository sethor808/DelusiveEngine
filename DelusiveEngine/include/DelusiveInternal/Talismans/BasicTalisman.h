#pragma once
#include <DelusiveInternal/Talismans/Talisman.h>

class BasicTalisman : public Talisman{
	std::string GetType() override { return "Basic"; }

};