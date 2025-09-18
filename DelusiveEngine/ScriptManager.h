#pragma once
#include <Delusive/ScriptRegistry.h>
#include "DelusiveScriptAPI.h"
#include <string>

class ScriptManager {
public:
	void Initialize() {
		InitializeScripts(registry);
	}
	BehaviourScript* CreateScript(const std::string& name, DelusiveScriptAgent* owner) {
		return registry.Create(name, owner);
	}
	void GetAvailableScripts(std::vector<std::string>& out) {
		registry.GetAllNames(out);
	}
private:
	ScriptRegistry registry;
};