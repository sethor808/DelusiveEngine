#pragma once
#include <Delusive/Runtime/Talismans/Talisman.h>

class BasicTalisman : public Talisman {
public:
	const char* GetType() const override { return "BasicTalisman"; }

};