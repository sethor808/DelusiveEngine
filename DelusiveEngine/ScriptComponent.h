#pragma once
#include "Component.h"
#include "ScriptManager.h"
#include "BehaviourScript.h"

struct DelusiveScript;
class DelusiveScriptAgent;

class ScriptComponent : public Component {
public:
    ScriptComponent(ScriptManager& scriptManager);

    ScriptComponent(const Component&) = delete;
    ScriptComponent& operator=(const ScriptComponent&) = delete;
    ScriptComponent(ScriptComponent&&) noexcept = default;
    ScriptComponent& operator=(ScriptComponent&&) noexcept = default;

    void RegisterProperties() override;

    const char* GetType() const override { return "ScriptComponent"; }
    std::unique_ptr<Component> Clone() const override;

    void SetOwner(Agent* agent) override;

    //ScriptManager is fetched from GameManager
    void AttachScript();

    void Update(float) override;

    // --- Serialization helpers ---
    std::string GetScriptName() const { return name; }
    void SetScriptName(const std::string& scriptName) { name = scriptName; }

    void Deserialize(std::istream& in) override;
private:
    ScriptManager& scriptManager;
	std::unique_ptr<DelusiveScriptAgent> scriptAgent;
    std::unique_ptr<DelusiveScript> scriptContainer;
};