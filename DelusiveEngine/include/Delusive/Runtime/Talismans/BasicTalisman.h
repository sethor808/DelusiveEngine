#pragma once
#include <Delusive/Runtime/Talismans/Talisman.h>

class BasicTalisman : public Talisman{
	std::string GetType() override { return "Basic"; }

};