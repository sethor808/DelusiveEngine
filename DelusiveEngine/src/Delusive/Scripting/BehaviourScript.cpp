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
    //type is filled in at save time - GetType() is pure virtual during base construction

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
    registry->type = GetType();
    registry->Serialize(out);
}

void BehaviourScript::Deserialize(DelusiveParser::DataBlock& in) {
    registry->Deserialize(in);
}
void BehaviourScript::Serialize(DelusiveParser::DataBlock& out) const {
    registry->type = GetType();
    registry->Serialize(out);
}

void BehaviourScript::CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const {
    DelusiveParser::DataBlock self;
    Serialize(self);
    self.id = id;
    out.push_back(std::move(self));
}
