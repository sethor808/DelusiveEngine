#pragma once
#include "Talisman.h"

class BasicTalisman : public Talisman{
	std::string GetType() override { return "Basic"; }

};