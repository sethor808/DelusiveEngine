#include <DelusiveExternal/DelusiveUICanvas.h>
#include <DelusiveExternal/DelusiveScriptAgent.h>
#include <DelusiveExternal/ScriptUIElement.h>
#include <DelusiveExternal/DelusiveScriptPlayer.h>
#include <Delusive/Runtime/UI/UICanvas.h>

std::string DelusiveUICanvas::GetName() const {
    return IsValid() ? canvas->GetName() : "Unlinked";
}

bool DelusiveUICanvas::IsActive() const {
    return IsValid() ? canvas->IsActive() : false;
}

void DelusiveUICanvas::SetActive(bool setting) {
    if(IsValid()) canvas->SetActive(setting);
}

DelusiveScriptPlayer DelusiveUICanvas::GetPlayer() const {
    DelusiveScriptPlayer temp;
    temp.Link(canvas->FetchPlayer());
    return temp;
}

ScriptUIElement DelusiveUICanvas::FindElementByUUID(const UUID& id) const {
    ScriptUIElement element;

    element.Link(canvas->FindElementByUUID(id));

    return element;
}