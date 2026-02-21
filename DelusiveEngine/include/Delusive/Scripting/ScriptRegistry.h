// ScriptRegistry.h
#pragma once
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <Delusive/Scripting/UIScript.h>
#include <Delusive/Scripting/BehaviourScript.h>
#include <iostream>

class BehaviourScript;

class ScriptRegistry {
public:
    using BehaviourFactory = std::function<std::unique_ptr<BehaviourScript>()>;
    using UIFactory = std::function<std::unique_ptr<UIScript>()>;

    ScriptRegistry() = default;

    void Init();

    void RegisterBehaviour(const std::string& name, BehaviourFactory factory) {
        enemyLogicFactories[name] = std::move(factory);
        
        if (loggingEnabled) {
            std::cout << "[ScriptRegistry] Registered BehaviourScript: " << name << "\n";
        }
    }

    void RegisterUIScript(const std::string& name, UIFactory factory) {
        uiFactories[name] = std::move(factory);

        if (loggingEnabled) {
            std::cout << "[ScriptRegistry] Registered UIScript: " << name << "\n";
        }
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
    bool loggingEnabled = true; //TODO: centralize this for a debug switch somewhere
};