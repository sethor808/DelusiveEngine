#include <Delusive/Runtime/Components/TransformComponent.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Scene/Scene.h>

void DrawIDLinkUI(DelusiveIDLink* link, const std::string& name, Scene* scene)
{
    Agent* resolved = nullptr;

    if (link->id.IsValid() && scene) {
        resolved = scene->FindAgentByUUID(link->id);
    }

    std::string displayName = resolved
        ? resolved->GetName()
        : (link->id.IsValid() ? "<Missing Agent>" : "<None>");

    ImGui::Button((name + ": " + displayName).c_str());

    //Allows drop payload
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload("DND_AGENT_UUID"))
        {
            UUID droppedUUID =
                *reinterpret_cast<const UUID*>(payload->Data);

            link->id = droppedUUID;
            link->dirty = true;
        }

        ImGui::EndDragDropTarget();
    }

    //Right-click to clear
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Clear"))
        {
            link->id = UUID{};
        }
        ImGui::EndPopup();
    }
}