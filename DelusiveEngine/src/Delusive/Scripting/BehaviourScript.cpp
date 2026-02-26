#include <Delusive/Scripting/BehaviourScript.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>

BehaviourScript::BehaviourScript() : 
    registry(std::make_unique<PropertyRegistry>()) {
    RegisterProperties();
}

BehaviourScript::~BehaviourScript() = default;

void BehaviourScript::RegisterProperties() {
    registry->Register("targetID", &targetID);
    registry->Register("movementSpeed", &movementSpeed);
}

void BehaviourScript::SetOwner(Agent* owner) {
    this->owner = owner;

    if (!owner) return;

    targetID.sceneLink = owner->GetScene();
}

void BehaviourScript::SetTarget(Agent* agent) {
    target = agent;
    targetID.id = agent ? agent->GetID() : UUID{};
}

UUID BehaviourScript::GetTargetID() const {
    return targetID.id;
}

void BehaviourScript::CopyCore(const BehaviourScript* base) {
    targetID.id = base->targetID.id;
    movementSpeed = base->movementSpeed;

    RelocateReferences();
}

void BehaviourScript::RelocateReferences() {
    if (targetID.id.IsValid() && owner) {
        target = owner->GetScene()->FindAgentByUUID(targetID.id);
    }
    else {
        target = nullptr;
    }
}

void BehaviourScript::DrawImGui() {
    if (!owner) {
        //Draw fallback?
        return;
    }

    registry->DrawImGui();
}

void BehaviourScript::Serialize(std::ostream& out) const {
    registry->Serialize(out);
}

void BehaviourScript::Deserialize(std::istream& in) {
    registry->Deserialize(in);
}