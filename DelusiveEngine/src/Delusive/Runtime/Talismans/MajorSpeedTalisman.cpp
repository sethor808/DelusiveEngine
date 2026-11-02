#include <Delusive/Runtime/Talismans/MajorSpeedTalisman.h>

void MajorSpeedTalisman::ApplyWhileActive(PlayerStats& stats) const {
	//Dodge speed follows moveSpeed
	stats.moveSpeed *= 1.25f;
}
