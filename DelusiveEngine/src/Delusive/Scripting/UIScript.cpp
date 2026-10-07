#include <Delusive/Scripting/UIScript.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>

UIScript::UIScript()
    : registry(std::make_unique<PropertyRegistry>())
{
    //Derived constructors call RegisterProperties - calling it here would
    //dispatch to the base GetType(), which is pure virtual during base construction
}

UIScript::~UIScript() = default;

void UIScript::RegisterProperties() {
    registry->category = "UIScript";
    //type is filled in at save time so it always names the most derived script

    registry->Register("id", &id);
}

void UIScript::DrawImGui() {
    registry->DrawImGui();
}
void UIScript::Deserialize(DelusiveParser::DataBlock& in) {
    registry->Deserialize(in);
}

void UIScript::Serialize(DelusiveParser::DataBlock& out) const {
    registry->type = GetType();
    registry->Serialize(out);
}

void UIScript::CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const {
    DelusiveParser::DataBlock self;
    Serialize(self);
    self.id = id;
    out.push_back(std::move(self));
}
