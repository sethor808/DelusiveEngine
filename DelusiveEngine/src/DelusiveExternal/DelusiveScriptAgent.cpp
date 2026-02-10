#include <DelusiveExternal/DelusiveScriptAgent.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Components/TransformComponent.h>

DelusiveScriptAgent::DelusiveScriptAgent(Agent* agent)
	: agent(agent) {
	transform = &agent->GetTransform();
}

uint64_t DelusiveScriptAgent::GetID() const {
    return agent->GetID();
}

const std::string& DelusiveScriptAgent::GetName() const {
    return agent->GetName();
}