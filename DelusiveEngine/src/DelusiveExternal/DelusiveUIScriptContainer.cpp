#include <DelusiveExternal/DelusiveUIScriptContainer.h>
#include <Delusive/Runtime/UI/UIScriptContainer.h>

UIScript* DelusiveUIScriptContainer::GetScript() const {
    return IsValid() ? rootContainer->GetScript() : nullptr;
}

void DelusiveUIScriptContainer::SetBinding(const std::string& name, const UUID& id) {
    if (!rootContainer) return;
    rootContainer->SetBinding(name, id);
}

bool DelusiveUIScriptContainer::HasBinding(const std::string& name) const {
    return rootContainer && rootContainer->HasBinding(name);
}

UUID DelusiveUIScriptContainer::GetBinding(const std::string& name) const {
    if (!rootContainer) return {};
    return rootContainer->GetBinding(name);
}

DelusiveUICanvas DelusiveUIScriptContainer::GetCanvas() {
    DelusiveUICanvas canvas;

    canvas.Link(rootContainer->GetCanvas());

    return canvas;
}