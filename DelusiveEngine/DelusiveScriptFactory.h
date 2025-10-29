#include <DelusiveExternal/ScriptFactoryBase.h>
#include <DelusiveExternal/UIScript.h>
#include "BehaviourScript.h"
#include <filesystem>
#include <unordered_map>
#include <iostream>

namespace fs = std::filesystem;

enum ScriptType {
	EnemyLogic,
	UI,
};

class DelusiveScriptFactory : public ScriptFactoryBase {
public:
	DelusiveScriptFactory() {
		DiscoverScripts();
	}

	std::unique_ptr<BehaviourScript> CreateEnemyLogic(const std::string& name) override;
	std::unique_ptr<UIScript> CreateUI(const std::string& name) override;

	const char** ListEnemyLogicScripts(size_t&) const override;
	const char** ListUIScripts(size_t&) const override;

private:
	std::unordered_map <std::string, std::function<std::unique_ptr<BehaviourScript()>> enemyLogicConstructors;
	std::unordered_map <std::string, std::function<std::unique_ptr_ptr<UIScript>()>> uiConstructors;

	mutable std::vector<const char*> enemyLogicCache;
	mutable std::vector<const char*> uiCache;

	void DiscoverScripts();
	void DiscoverDirectory(const std::string&, ScriptType)
};