// ScriptManager.h
#pragma once
#include <Delusive/Scripting/ScriptRegistry.h>
#include <memory>

class ScriptManager {
public:
    void Init() { registry.Init(); }

    std::unique_ptr<BehaviourScript> CreateEnemyLogicScript(const std::string& name) {
        return registry.CreateEnemyLogic(name);
    }

    std::unique_ptr<UIScript> CreateUIScript(const std::string& name) {
        return registry.CreateUIScript(name);
    }

    void GetAvailableEnemyLogicScripts(std::vector<std::string>& out) {
        registry.GetAllEnemyLogicNames(out);
    }

    void GetAvailableUIScripts(std::vector<std::string>& out) {
        registry.GetAllUINames(out);
    }

private:
    ScriptRegistry registry;
};