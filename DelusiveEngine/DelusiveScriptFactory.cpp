#include "DelusiveScriptFactory.h"

std::unique_ptr<BehaviourScript> DelusiveScriptFactory::CreateEnemyLogic(const std::string& name) {
	auto it = behaviourConstructors.find(name);
	if (it != behaviourConstructors.end()) return it->second();
	return nullptr;
}

std::unique_ptr<UIScript> CreateUI(const std::string& name) {
	auto it = uiConstructors.find(name);
	if (it != uiConstructors.end()) return it->second();
	return nullptr;
}

const char** DelusiveScriptFactory::ListEnemyLogicScripts(size_t& count) const {
	enemyLogicCache.clear;
	for (auto& [k, _] : enemyLogicConstructors) enemyLogicCache.push_back(k.c_str());
	count = enemyLogicCache.size();
	return enemyLogicCache.data();
}

void DelusiveScriptFactory::DiscoverScripts() {
	DiscoverDirectory("scripts/enemyLogic", ScriptType::EnemyLogic);
	DiscoverDirectory("scripts/ui", ScriptType::UI);
}

void DelusiveScriptFactory::DiscoverDirectory(cont std::string& path, ScriptType type) {
	for (const auto& entry : fs::directory_iterator(dir)) {
		if (!entry.is_regular_file()) continue;
		std::string file = entry.path().stem().string();

		if (type == ScriptType::UI) {
			uiConstructors[file] = [file]() {
				return std::make_unique<UIScript>(); // later replaced with real dynamic link
				};
		}
		else if (type == ScriptType::EnemyLogic){
			enemyLogicConstructors[file] = [file]() {
				return std::make_unique<BehaviourScript>(); // later replaced
				};
		}
	}
}

extern "C" __declspec(dllexport)
ScriptFactoryBase* GetDelusiveScriptFactory() {
	static DelusiveScriptFactory instance();
	return &instance;
}