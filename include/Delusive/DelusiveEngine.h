#pragma once
#include <memory>

namespace DelusiveEngine
{
	struct DelusiveContext {
		bool editorMode = false;
		int windowWidth = 1920;
		int windowHeight = 1080;
		const char* windowTitle = "Delusive Editor";
	};

	int Run(const DelusiveContext&);
	void Shutdown();
}