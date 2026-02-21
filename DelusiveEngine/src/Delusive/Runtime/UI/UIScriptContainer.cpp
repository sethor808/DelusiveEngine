#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <Delusive/Scripting/UIScript.h>

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
        script->Link(this);
        script->OnInit();
    }
}

void UIScriptContainer::SetBinding(const std::string& name, const UUID& id)
{
    bindings[name] = id;
}

bool UIScriptContainer::HasBinding(const std::string& name) const
{
    auto it = bindings.find(name);
    return it != bindings.end() && it->second.IsValid();
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