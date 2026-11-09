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

void WorldClock::SetTimeScale(const std::string& source, float scale) {
	std::erase_if(scales, [&](const Scale& s) { return s.source == source; });
	scale = std::clamp(scale, 0.0f, 1.0f);
	if (scale < 1.0f) scales.push_back({ source, scale });
}

float WorldClock::GetTimeScale() const {
	float total = 1.0f;
	for (const Scale& s : scales) total *= s.scale;
	return total;
}

bool WorldClock::Advance() {
	if (IsHeld()) return false;
	if (freezeTicks > 0) {
		--freezeTicks;
		return false;
	}

	scaledTicks += GetTimeScale();
	if (scaledTicks < 1.0f) return false;
	scaledTicks -= 1.0f;
	return true;
}

void WorldClock::Reset() {
	holds.clear();
	scales.clear();
	freezeTicks = 0;
	scaledTicks = 0.0f;
}
