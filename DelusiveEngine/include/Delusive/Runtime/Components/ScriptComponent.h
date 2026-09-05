#pragma once
#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>


class Agent;
class BehaviourScript;

class ScriptComponent : public Component {
public:
    ScriptComponent(DelusiveInstance&);
    ScriptComponent() = delete;

    ScriptComponent(const Component&) = delete;
    ScriptComponent& operator=(const ScriptComponent&) = delete;
    ScriptComponent(ScriptComponent&&) noexcept = default;
    ScriptComponent& operator=(ScriptComponent&&) noexcept = default;

    void InitScript();

    void RegisterProperties() override;

    void SetOwner(Agent*) override;
    const char* GetType() const override { return "ScriptComponent"; }
    std::unique_ptr<Component> Clone() const override;

    void SetTarget();
    void SetTarget(Agent*);

    void Update(float) override;
    void DrawImGui() override;

    BehaviourScript* GetScript() const;

    // --- Serialization helpers ---
    std::string GetScriptName() const { return name; }
    void SetScriptName(const std::string& scriptName) { name = scriptName; }
private:
    DelusiveLink<Agent> target;
    DelusiveObject<BehaviourScript> script;
};