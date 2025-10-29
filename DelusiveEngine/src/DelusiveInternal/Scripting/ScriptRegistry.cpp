// ScriptRegistry.cpp
#include <DelusiveExternal/ScriptRegistry.h>
#include <DelusiveExternal/UIScript.h>
#include <iostream>
#include <filesystem>

typedef void (*InitializeScriptsFn)(ScriptRegistry&);
typedef void (*ShutdownScriptsFn)();

std::unique_ptr<BehaviourScript> ScriptRegistry::CreateEnemyLogic(const std::string& name) const {
    auto it = enemyLogicFactories.find(name);
    if (it != enemyLogicFactories.end())
        return it->second();
    return nullptr;
}

std::unique_ptr<UIScript> ScriptRegistry::CreateUIScript(const std::string& name) const {
    auto it = uiFactories.find(name);
    if (it != uiFactories.end())
        return it->second();
    return nullptr;
}