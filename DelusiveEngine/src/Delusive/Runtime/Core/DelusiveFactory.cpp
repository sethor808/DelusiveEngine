#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Components/DelusiveComponentFactory.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <Delusive/Runtime/UI/DelusiveUI.h>

//Complete types are required here - unique_ptr destruction needs them
#include <Delusive/Runtime/Components/Component.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Scripting/BehaviourScript.h>
#include <Delusive/Scripting/UIScript.h>

std::unique_ptr<Component> DelusiveFactory<Component>::Create(const std::string& type, DelusiveInstance& instance) {
    return DelusiveComponentFactory::CreateComponentByType(type, instance);
}

std::unique_ptr<UIElement> DelusiveFactory<UIElement>::Create(const std::string& type, DelusiveInstance& instance) {
    return DelusiveUI::CreateUIElementByType(type, instance);
}

std::unique_ptr<BehaviourScript> DelusiveFactory<BehaviourScript>::Create(const std::string& type, DelusiveInstance& instance) {
    return instance.scriptManager.CreateEnemyLogicScript(type);
}

std::unique_ptr<UIScript> DelusiveFactory<UIScript>::Create(const std::string& type, DelusiveInstance& instance) {
    return instance.scriptManager.CreateUIScript(type);
}
