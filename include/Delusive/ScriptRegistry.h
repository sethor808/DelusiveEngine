#pragma once
#include <string>
#include <map>
#include <functional>
#include <vector>
#include <memory>
#include "BehaviourScript.h"
#include "DelusiveScriptAgent.h"

// Factory function that creates a BehaviourScript for a given owner
using ScriptFactoryFunc = std::function<BehaviourScript* (DelusiveScriptAgent*)>;

class ScriptRegistry {
public:
    void Register(const std::string& name, ScriptFactoryFunc factory) {
        registry[name] = factory;
    }

    BehaviourScript* Create(const std::string& name, DelusiveScriptAgent* owner) const {
        auto it = registry.find(name);
        return it != registry.end() ? it->second(owner) : nullptr;
    }

    void GetAllNames(std::vector<std::string>& out) const {
        out.clear();
        for (auto& pair : registry) out.push_back(pair.first);
    }

private:
    std::map<std::string, ScriptFactoryFunc> registry;
};