#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <iostream>

#include "BehaviourScript.h"

class ScriptRegistry {
public:
	using ScriptFactory = std::function < std::unique_ptr<BehaviourScript>()>;

	static ScriptRegistry& Instance() {
		static ScriptRegistry instance;
		return instance;
	}

	void Register(const std::string& name, ScriptFactory factory) {
		factories[name] = factory;
	}

	std::unique_ptr<BehaviourScript> Create(const std::string& name) {
		auto it = factories.find(name);
		if (it != factories.end()) {
			return it->second();
		}

		std::cerr << "[ScriptRegistry] Could not find script: " << name << std::endl;
		return nullptr;
	}

	std::vector<std::string> GetNames() const {
		std::vector<std::string> names;
		for (auto& key : factories) {
			names.push_back(key.first);
		}
		return names;
	}
private:
	std::unordered_map<std::string, ScriptFactory> factories;
};

// Macro for auto-registration
#define REGISTER_SCRIPT(ScriptClass) \
    static bool ScriptClass##_registered = [](){ \
        ScriptRegistry::Instance().Register(#ScriptClass, [](){ \
            return std::make_unique<ScriptClass>(); \
        }); \
        return true; \
    }()