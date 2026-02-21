#include <Delusive/Runtime/Talismans/Talisman.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>

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

std::string Talisman::GetBaseTexture() const {
	return talismanBase;
}

std::string Talisman::GetGlyphTexture() const {
	return talismanGlyph;
}

std::string Talisman::GetStringTexture() const {
	return TALISMAN_STRING;
}