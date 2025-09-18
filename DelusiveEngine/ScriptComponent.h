#pragma once
#include "Component.h"
#include "ScriptManager.h"
#include "BehaviourScript.h"

class ScriptComponent : public Component {
public:
    ScriptComponent() = default;
    ScriptComponent(const std::string& scriptName) { name = scriptName; }

    //ScriptManager is fetched from GameManager
    void AttachScript(ScriptManager& manager, DelusiveScriptAgent* owner) {
        if (!name.empty()) {
            script.reset(manager.CreateScript(name, owner));
        }
    }

    void Update(float deltaTime) override {
        if (script) script->Update(deltaTime);
    }

    // --- Serialization helpers ---
    std::string GetScriptName() const { return name; }
    void SetScriptName(const std::string& scriptName) { name = scriptName; }
private:
	std::unique_ptr<BehaviourScript> script;
};