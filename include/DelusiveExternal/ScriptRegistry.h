// ScriptRegistry.h
#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "UIScript.h"
#include <DelusiveExternal/BehaviourScript.h>

class BehaviourScript;

class ScriptRegistry {
public:
    using BehaviourFactory = std::function<std::unique_ptr<BehaviourScript>()>;
    using UIFactory = std::function<std::unique_ptr<UIScript>()>;

    ScriptRegistry() = default;

    void RegisterBehaviour(const std::string& name, BehaviourFactory factory) {
        enemyLogicFactories[name] = std::move(factory);
    }

    void RegisterUIScript(const std::string& name, UIFactory factory) {
        uiFactories[name] = std::move(factory);
    }

    std::unique_ptr<BehaviourScript> CreateEnemyLogic(const std::string& name) const;
    std::unique_ptr<UIScript> CreateUIScript(const std::string&) const;

    void GetAllEnemyLogicNames(std::vector<std::string>& out) const {
        out.clear();
        out.reserve(enemyLogicFactories.size());
        for (const auto& entry : enemyLogicFactories)
            out.push_back(entry.first);
    }

    void GetAllUINames(std::vector<std::string>& out) const {
        out.clear();
        out.reserve(uiFactories.size());
        for (const auto& entry : uiFactories)
            out.push_back(entry.first);
    }

private:
    std::unordered_map<std::string, BehaviourFactory> enemyLogicFactories;
    std::unordered_map<std::string, UIFactory> uiFactories;
};