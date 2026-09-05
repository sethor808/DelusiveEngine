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
}

void UIScript::DrawImGui() {
    registry->DrawImGui();
}