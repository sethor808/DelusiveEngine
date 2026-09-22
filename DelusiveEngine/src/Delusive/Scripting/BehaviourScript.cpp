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
    registry->category = "BehaviourScript";
    registry->type = GetType();

    registry->Register("id", &id);
    registry->Register("target", &target);
    registry->Register("movementSpeed", &movementSpeed);
}

void BehaviourScript::Update(float deltaTime) {
    if (target.dirty) {
        RelocateReferences();
    }
}

void BehaviourScript::SetOwner(Agent* newOwner) {
    owner = newOwner;
}

void BehaviourScript::SetTarget(Agent* agent) {
    target = agent;
    target.dirty = true;
}

UUID BehaviourScript::GetTargetID() const {
    return target.getID();
}

void BehaviourScript::CopyCore(const BehaviourScript* base) {
    //A clone inherits the id but never the resolved pointer
    target.id = base->target.id;
    target.cached = nullptr;
    target.dirty = true;

    movementSpeed = base->movementSpeed;

    RelocateReferences();
}

void BehaviourScript::RelocateReferences() {
    //No owner means no scene to search, so stay dirty and retry once one is attached
    Scene* scene = owner ? owner->GetScene() : nullptr;
    if (!scene) return;

    //Assign cached directly - set() would wipe the id when the lookup misses
    target.cached = target.id.IsValid()
        ? scene->FindAgentByUUID(target.id)
        : nullptr;

    target.dirty = false;
}

void BehaviourScript::DrawImGui() {
    registry->DrawImGui();
}

void BehaviourScript::Serialize(std::ostream& out) const {
    registry->Serialize(out);
}

void BehaviourScript::Deserialize(DelusiveParser::DataBlock& in) {
    registry->Deserialize(in);
}
void BehaviourScript::Serialize(DelusiveParser::DataBlock& out) const {
    registry->Serialize(out);
}
