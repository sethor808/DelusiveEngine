#include <Delusive/Runtime/Components/ScriptComponent.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <Delusive/Scripting/BehaviourScript.h>
#include <iostream>

ScriptComponent::ScriptComponent(DelusiveInstance& instance)
	: Component(instance)
{ 
	name = "New ScriptComponent";

	RegisterProperties();
}

BehaviourScript* ScriptComponent::GetScript() const {
    return script.get();
}

void ScriptComponent::SetOwner(Agent* agent) {
    if (agent) {
        owner = agent;

        if (script) {
            script->SetOwner(owner);
            script->RelocateReferences();
        }

        //SetTarget();
    }
}

std::unique_ptr<Component> ScriptComponent::Clone() const {
	auto copy = std::make_unique<ScriptComponent>(instance);
	copy->SetName(GetName());
    if(script) copy->script.object = script->Clone();
	return copy;
}

void ScriptComponent::SetTarget() {
	if (!owner || !owner->GetScene()) return;

	Agent* player = owner->GetScene()->FetchPlayerRaw();
	if (player) {
		SetTarget(player);
	}
}

void ScriptComponent::SetTarget(Agent* agent) {
	if (!agent || !script.get()) return;

    target.cached = agent;
	script->SetTarget(agent);
}

void ScriptComponent::RegisterProperties()
{
	registry->Register("script", &script);
}

void ScriptComponent::Update(float deltaTime) {
	if (script) {
		script->Update(deltaTime);
	}
}

void ScriptComponent::DrawImGui() {
    Component::DrawImGui();

    /*
    if (scriptContainer->newScript) {
        AttachScript();
    }
    */
}