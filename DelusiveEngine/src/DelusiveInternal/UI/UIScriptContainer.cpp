#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <DelusiveExternal/UIScript.h>
#include <DelusiveExternal/ScriptUIElement.h>
#include <DelusiveExternal/DelusiveUIScriptContainer.h>

UIScriptContainer::UIScriptContainer(DelusiveRenderer& renderer) 
	: UIElement(renderer)
{

}

std::unique_ptr<UIElement> UIScriptContainer::Clone() const{
	return std::make_unique<UIScriptContainer>(renderer);
}

void UIScriptContainer::Update(float deltaTime) {
    UIElement::Update(deltaTime);
    if (script) script->OnUpdate(deltaTime);
}

void UIScriptContainer::SetScript(std::unique_ptr<UIScript> newScript) {
    script = std::move(newScript);
    if (script) {
        DelusiveUIScriptContainer newElement;
        newElement.Link(this);
        script->Link(&newElement);
        script->OnInit();
    }
}

void UIScriptContainer::DrawImGui() {
    ImGui::Separator();
    ImGui::Text("Script Bindings");

    for (auto& [name, uuid] : bindings) {
        ImGui::PushID(name.c_str());

        ImGui::Text("%s", name.c_str());
        ImGui::SameLine();

        ImGui::TextDisabled("%s", uuid.ToString().c_str());

        // Drag-drop target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("UI_ELEMENT_UUID")) {
                UUID dropped;
                memcpy(&dropped, payload->Data, sizeof(UUID));
                uuid = dropped;
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            uuid = UUID{}; // invalid / empty
        }

        ImGui::PopID();
    }


    //Do the regular display underneath
    ImGui::Separator();
    UIElement::DrawImGui();
}