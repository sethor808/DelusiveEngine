#include "Talisman.h"

bool Talisman::TakeDamage() {
	hp -= 1;

	if (hp <= 0) {
		isBroken = true;
		return isBroken;
	} else {
		isBroken = false;
		return isBroken;
	}
}