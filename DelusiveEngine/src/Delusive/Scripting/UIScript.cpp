#include <Delusive/Scripting/UIScript.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>

UIScript::UIScript() {
    RegisterProperties();
}

UIScript::~UIScript() = default;

void UIScript::Serialize(std::ostream& os) const {
    registry->Serialize(os);
}

void UIScript::Deserialize(std::istream& is) {
    registry->Deserialize(is);
}

void UIScript::DrawImGui() {
    registry->DrawImGui();
}