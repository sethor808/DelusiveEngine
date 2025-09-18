#include <Delusive/DelusiveScriptAgent.h>
#include "Agent.h"
#include "TransformComponent.h"

DelusiveScriptAgent::DelusiveScriptAgent(Agent* agent)
	: agent(agent) {

}

uint64_t DelusiveScriptAgent::GetID() const {
    return agent->GetID();
}

const std::string& DelusiveScriptAgent::GetName() const {
    return agent->GetName();
}