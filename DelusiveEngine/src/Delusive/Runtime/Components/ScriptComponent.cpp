#include <Delusive/Runtime/Components/ScriptComponent.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <iostream>

ScriptComponent::ScriptComponent(DelusiveRenderer& renderer, ScriptManager& scriptManager)
	: Component(renderer), scriptManager(scriptManager)
{ 
	name = "New ScriptComponent";
	InitScript();

	RegisterProperties();
}

void ScriptComponent::SetOwner(Agent* agent) {
    if (agent) {
        owner = agent;
        AttachScript();
        SetTarget();
    }
}

void ScriptComponent::InitScript() {
	scriptContainer = std::make_unique<DelusiveScript>();
	std::cout << "[ScriptComponent] Initialized script container." << std::endl;
	scriptContainer->manager = &scriptManager;
}

std::unique_ptr<Component> ScriptComponent::Clone() const {
	auto copy = std::make_unique<ScriptComponent>(renderer, scriptManager);
	copy->SetName(GetName());
	copy->scriptContainer->scriptName = scriptContainer->scriptName;
    copy->scriptContainer->script = scriptContainer->script->Clone();
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
	if (!agent || !scriptContainer || !scriptContainer->script) return;

	target = agent;
	scriptContainer->script->SetTarget(agent);
}

void ScriptComponent::RegisterProperties()
{
	registry->Register("script", scriptContainer.get());
}

void ScriptComponent::AttachScript() {
	std::cout << "[ScriptComponent] Attempting to attach script." << std::endl;
    auto newScript = scriptManager.CreateEnemyLogicScript(scriptContainer->scriptName);
    if (newScript) {
        scriptContainer->script = std::move(newScript);
        scriptContainer->script->SetOwner(owner);
        std::cout << "[ScriptComponent] Attached script: " << scriptContainer->scriptName << std::endl;
    }
}

void ScriptComponent::Update(float deltaTime) {
	if (scriptContainer->script.get()) {
		scriptContainer->script->Update(deltaTime);
	}
}

void ScriptComponent::Deserialize(std::istream& in) {
	Component::Deserialize(in);
	AttachScript();
}