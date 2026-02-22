#include <Delusive/Scripting/BehaviourScript.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Scene/Scene.h>

void BehaviourScript::SetTarget(Agent* agent) {
    target = agent;
    targetID = agent ? agent->GetID() : UUID{};
}

void BehaviourScript::CopyCore(const BehaviourScript* base) {
    targetID = base->targetID;
    movementSpeed = base->movementSpeed;

    RelocateReferences();
}

void BehaviourScript::RelocateReferences() {
    if (targetID.IsValid() && owner) {
        target = owner->GetScene()->FindAgentByUUID(targetID);
    }
    else {
        target = nullptr;
    }
}