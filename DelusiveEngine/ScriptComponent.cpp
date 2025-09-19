#include "ScriptComponent.h"
#include "DelusiveData.h"

ScriptComponent::ScriptComponent(ScriptManager& scriptManager)
	: scriptManager(scriptManager)
{ 
	name = "New ScriptComponent";
	if (owner) {
		scriptAgent = std::make_unique<DelusiveScriptAgent>(owner);
	}
	scriptContainer = std::make_unique<DelusiveScript>();
	scriptContainer->manager = &scriptManager;
	RegisterProperties();
}

std::unique_ptr<Component> ScriptComponent::Clone() const {
	auto script = std::make_unique<ScriptComponent>(scriptManager);
	script->SetName(GetName());
	script->scriptContainer->scriptName = scriptContainer->scriptName;
	script->SetOwner(owner);
	return script;
}

void ScriptComponent::SetOwner(Agent* agent) {
	Component::SetOwner(agent);
	if (owner) {
		scriptAgent = std::make_unique<DelusiveScriptAgent>(owner);
	}
}

void ScriptComponent::RegisterProperties()
{
	registry->Register("script", scriptContainer.get());
}

void ScriptComponent::AttachScript() {
	if (!scriptContainer->scriptName.empty() && scriptContainer->script.get()) {
		scriptContainer->script.reset(scriptManager.CreateScript(scriptContainer->scriptName, scriptAgent.get()));
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