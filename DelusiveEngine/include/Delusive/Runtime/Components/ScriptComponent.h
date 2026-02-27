#pragma once
#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <Delusive/Scripting/BehaviourScript.h>

struct DelusiveScript;
class Agent;

class ScriptComponent : public Component {
public:
    ScriptComponent(DelusiveRenderer&, ScriptManager&);
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

    //ScriptManager is fetched from GameManager
    void AttachScript();

    void Update(float) override;
    void DrawImGui() override;

    DelusiveScript* GetScriptContainer() { return scriptContainer.get(); }

    // --- Serialization helpers ---
    std::string GetScriptName() const { return name; }
    void SetScriptName(const std::string& scriptName) { name = scriptName; }

    void Deserialize(std::istream& in) override;
private:
    ScriptManager& scriptManager;
    Agent* target;
    std::unique_ptr<DelusiveScript> scriptContainer;
};