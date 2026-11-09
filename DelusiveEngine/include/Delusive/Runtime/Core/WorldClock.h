#pragma once
#include <string>
#include <vector>

//Decides whether a scene's world advances each tick. Every stop goes through here:
//holds (pause menus, cutscenes) last until their source releases them; freezes (hit
//stop) last a number of ticks. Time scale slows the world by skipping ticks, so frame
//data plays exactly as authored, just stretched. The camera and UI keep running either way.
class WorldClock {
public:
	//Held stops - one per source name, so two sources never release each other
	void SetHold(const std::string& source, bool held);
	bool IsHeld() const { return !holds.empty(); }

	//Timed stop, rounded to whole ticks; a shorter freeze never cuts a longer one short
	void Freeze(float seconds);
	bool IsFrozen() const { return freezeTicks > 0; }

	bool IsStopped() const { return IsHeld() || IsFrozen(); }

	//Slow motion per source, 0-1; sources multiply and 1 removes the source
	void SetTimeScale(const std::string& source, float scale);
	float GetTimeScale() const;

	//Once per tick: true if the world advances. A held clock keeps its freeze for later;
	//freezes count real ticks, so slow motion never stretches a hit stop
	bool Advance();
	void Reset();

private:
	struct Scale {
		std::string source;
		float scale;
	};

	std::vector<std::string> holds;
	std::vector<Scale> scales;
	int freezeTicks = 0;
	//Builds up by the time scale each tick; the world steps when it reaches one
	float scaledTicks = 0.0f;
};
