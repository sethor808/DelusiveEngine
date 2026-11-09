#pragma once
#include <string>
#include <vector>

//Decides whether a scene's world advances each tick. Every stop goes through here:
//holds (pause menus, cutscenes) last until their source releases them; freezes (hit
//stop) last a number of ticks. The camera and UI keep running either way.
class WorldClock {
public:
	//Held stops - one per source name, so two sources never release each other
	void SetHold(const std::string& source, bool held);
	bool IsHeld() const { return !holds.empty(); }

	//Timed stop, rounded to whole ticks; a shorter freeze never cuts a longer one short
	void Freeze(float seconds);
	bool IsFrozen() const { return freezeTicks > 0; }

	bool IsStopped() const { return IsHeld() || IsFrozen(); }

	//Once per tick: true if the world advances. A held clock keeps its freeze for later
	bool Advance();
	void Reset();

private:
	std::vector<std::string> holds;
	int freezeTicks = 0;
};
