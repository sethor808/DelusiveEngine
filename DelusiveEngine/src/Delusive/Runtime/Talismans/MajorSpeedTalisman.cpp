#include <Delusive/Runtime/Talismans/MajorSpeedTalisman.h>

void MajorSpeedTalisman::ApplyWhileActive(PlayerStats& stats) const {
	stats.moveSpeed *= 1.25f;
	stats.dodgeStrength *= 1.25f;
}
