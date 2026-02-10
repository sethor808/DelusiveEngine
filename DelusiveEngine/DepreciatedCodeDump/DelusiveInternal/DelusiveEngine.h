#pragma once
#include <memory>

namespace DelusiveEngine
{
	struct DelusiveContext {
		bool editorMode = false;
		int windowWidth = 1280;
		int windowHeight = 720;
		const char* windowTitle = "Delusive Editor";
	};

	int Run(const DelusiveContext&);
	void Shutdown();
}