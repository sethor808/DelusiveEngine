#include "ScriptComponent.h"
#include "Agent.h"
#include "Scene.h"
#include "DelusiveData.h"
#include <iostream>

ScriptComponent::ScriptComponent(ScriptManager& scriptManager)
	: scriptManager(scriptManager)
{ 
	name = "New ScriptComponent";
	InitScript();

	RegisterProperties();
}

void ScriptComponent::InitScript() {
	scriptContainer = std::make_unique<DelusiveScript>();
	std::cout << "[ScriptComponent] Initialized script container." << std::endl;
	scriptContainer->manager = &scriptManager;
}

std::unique_ptr<Component> ScriptComponent::Clone() const {
	auto copy = std::make_unique<ScriptComponent>(scriptManager);
	copy->SetName(GetName());
	copy->scriptContainer->scriptName = scriptContainer->scriptName;
	return copy;
}

void ScriptComponent::SetOwner(Agent* agent) {
	if (agent) {
		owner = agent;
		scriptAgent = std::make_unique<DelusiveScriptAgent>(owner);
		AttachScript();
		SetTarget();
	}
}

void ScriptComponent::SetTarget() {
	if (!owner || !owner->GetScene()) return;

	Agent* player = owner->GetScene()->FetchPlayer();
	if (player) {
		SetTarget(player);
	}
}

void ScriptComponent::SetTarget(Agent* agent) {
	if (!agent || !scriptContainer || !scriptContainer->script) return;

	target = std::make_unique<DelusiveScriptAgent>(agent);
	scriptContainer->script->SetTarget(target.get());
}

void ScriptComponent::RegisterProperties()
{
	registry->Register("script", scriptContainer.get());
}

void ScriptComponent::AttachScript() {
	std::cout << "[ScriptComponent] Attempting to attach script." << std::endl;
	if (!scriptContainer->scriptName.empty() && !scriptContainer->script) {
		scriptContainer->script.reset(scriptManager.CreateScript(scriptContainer->scriptName, scriptAgent.get()));
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