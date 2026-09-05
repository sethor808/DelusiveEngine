#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <Delusive/Scripting/UIScript.h>

UIScriptContainer::UIScriptContainer(DelusiveInstance& instance)
	: UIElement(instance)
{
    RegisterProperties();
}

void UIScriptContainer::LinkCanvas(UICanvas* link) {
    UIElement::LinkCanvas(link);

    if (script) {
        script->Link(this);
    }
}

void UIScriptContainer::RegisterProperties() {
    UIElement::RegisterProperties();

    registry->Register("script", &script);
}

std::unique_ptr<UIElement> UIScriptContainer::Clone() const{
	return std::make_unique<UIScriptContainer>(instance);
}

void UIScriptContainer::Update(float deltaTime) {
    UIElement::Update(deltaTime);
    if (script) script->OnUpdate(deltaTime);
}

void UIScriptContainer::SetScript(std::unique_ptr<UIScript> newScript) {
    script.object = std::move(newScript);
    if (script) {
        script->Link(this);
        script->OnInit();
    }
}

void UIScriptContainer::DrawImGui() {
    UIElement::DrawImGui();

    /*
    if (scriptContainer->newScript) {
        AttachScript();
    }
    */
}