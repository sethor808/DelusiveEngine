#pragma once

#ifdef DELUSIVESCRIPTS_EXPORTS
	#define DS_API __declspec(dllexport)
#else
	#define DS_API __declspec(dllimport)
#endif

#include <Delusive/ScriptRegistry.h>

extern "C" {
	DS_API void InitializeScripts(ScriptRegistry& registry);
	DS_API void UpdateScripts(float deltaTime);
	DS_API void ShutdownScripts();
}