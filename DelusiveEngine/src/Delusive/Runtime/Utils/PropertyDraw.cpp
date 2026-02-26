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
}