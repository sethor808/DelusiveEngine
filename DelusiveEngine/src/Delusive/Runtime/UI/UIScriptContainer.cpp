#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <Delusive/Scripting/UIScript.h>

UIScriptContainer::UIScriptContainer(DelusiveRenderer& renderer, ScriptManager& manager) 
	: UIElement(renderer), scriptManager(manager)
{

}

std::unique_ptr<UIElement> UIScriptContainer::Clone() const{
	return std::make_unique<UIScriptContainer>(renderer, scriptManager);
}

void UIScriptContainer::Update(float deltaTime) {
    UIElement::Update(deltaTime);
    if (scriptContainer->script) scriptContainer->script->OnUpdate(deltaTime);
}

void UIScriptContainer::SetScript(std::unique_ptr<UIScript> newScript) {
    scriptContainer->script = std::move(newScript);
    if (scriptContainer->script) {
        scriptContainer->script->Link(this);
        scriptContainer->script->OnInit();
    }
}

void UIScriptContainer::DrawImGui() {
    UIElement::DrawImGui();
}