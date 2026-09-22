#include <Delusive/Scripting/UIScript.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>

UIScript::UIScript()
    : registry(std::make_unique<PropertyRegistry>())
{
    RegisterProperties();
}

UIScript::~UIScript() = default;

void UIScript::RegisterProperties() {
    registry->category = "UIScript";
    registry->type = GetType();

    registry->Register("id", &id);
}

void UIScript::DrawImGui() {
    registry->DrawImGui();
}
void UIScript::Deserialize(DelusiveParser::DataBlock& in) {
    registry->Deserialize(in);
}

void UIScript::Serialize(DelusiveParser::DataBlock& out) const {
    registry->Serialize(out);
}
