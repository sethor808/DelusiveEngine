#pragma once
#include <memory>

namespace DelusiveEngine
{
	struct DelusiveContext {
		bool editorMode = false;
		int windowWidth = 1280;
		int windowHeight = 720;
		const char* windowTitle = "Delusive Editor";
		//Render rate cap, 0 = uncapped. Gameplay ticks stay at DELUSIVE_TICKS_PER_SECOND either way.
		int maxFPS = 120;
	};

	int Run(const DelusiveContext&);
	void Shutdown();
}