#pragma once
#include <Delusive/Runtime/Talismans/Talisman.h>

class BasicTalisman : public Talisman{
public:
	std::string GetType() const override { return "BasicTalisman"; }

};