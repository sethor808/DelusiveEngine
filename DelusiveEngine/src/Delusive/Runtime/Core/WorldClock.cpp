#include <Delusive/Runtime/Core/WorldClock.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <algorithm>
#include <cmath>

void WorldClock::SetHold(const std::string& source, bool held) {
	auto found = std::find(holds.begin(), holds.end(), source);
	if (held && found == holds.end()) holds.push_back(source);
	else if (!held && found != holds.end()) holds.erase(found);
}

void WorldClock::Freeze(float seconds) {
	const int ticks = static_cast<int>(std::lround(seconds * DELUSIVE_TICKS_PER_SECOND));
	freezeTicks = std::max(freezeTicks, ticks);
}

bool WorldClock::Advance() {
	if (IsHeld()) return false;
	if (freezeTicks > 0) {
		--freezeTicks;
		return false;
	}
	return true;
}

void WorldClock::Reset() {
	holds.clear();
	freezeTicks = 0;
}
