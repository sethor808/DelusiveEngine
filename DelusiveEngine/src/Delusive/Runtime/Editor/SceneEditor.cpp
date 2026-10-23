#include <Delusive/Runtime/Editor/SceneEditor.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Agents/DelusiveAgents.h>
#include <Delusive/Runtime/Components/DelusiveComponents.h>
#include <Delusive/Runtime/Components/DelusiveComponentFactory.h>
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Core/DelusiveClone.h>
#include <Delusive/Runtime/Core/PhysicsSystem.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Runtime/Utils/DelusiveUtils.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <imgui/imgui_internal.h>
#include <algorithm>
#include <cstdio>
#include <filesystem>

namespace {
    //Where an agent shows in the view: its sprites, else its colliders, else its own box
    bool AgentBounds(Agent& agent, glm::vec2& min, glm::vec2& max) {
        bool any = false;
        auto Add = [&](const glm::vec2& p) {
            if (!any) { min = max = p; any = true; }
            else { min = glm::min(min, p); max = glm::max(max, p); }
        };

        const glm::mat4 agentMatrix = agent.GetTransform().ToMatrix();
        for (SpriteComponent* sprite : agent.GetComponentsOfType<SpriteComponent>()) {
            if (!sprite->transform) continue;
            const glm::mat4 model = agentMatrix * sprite->transform->ToMatrix();
            for (float x : { -0.5f, 0.5f })
                for (float y : { -0.5f, 0.5f })
                    Add(glm::vec2(model * glm::vec4(x, y, 0.0f, 1.0f)));
        }
        if (!any) {
            for (ColliderComponent* collider : agent.GetComponentsOfType<ColliderComponent>()) {
                const WorldShape shape = PhysicsSystem::BuildShape(*collider);
                Add(shape.min);
                Add(shape.max);
            }
        }
        if (!any) {
            const Transform& t = agent.GetTransform();
            const glm::vec2 half = glm::max(glm::abs(t.scale) * 0.5f, glm::vec2(0.25f));
            Add(t.position - half);
            Add(t.position + half);
        }
        return any;
    }

    ImVec2 WorldToScreen(const glm::vec2& world, const glm::mat4& projection) {
        const glm::vec4 ndc = projection * glm::vec4(world, 0.0f, 1.0f);
        const ImVec2 display = ImGui::GetIO().DisplaySize;
        return ImVec2((ndc.x + 1.0f) * 0.5f * display.x, (1.0f - ndc.y) * 0.5f * display.y);
    }

    glm::vec2 MouseWorld(DelusiveInstance& instance) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        return ScreenToWorld2D((int)mouse.x, (int)mouse.y, instance.renderer.GetProjection());
    }

    std::string AgentLabel(const Agent& agent) {
        return agent.GetName().empty() ? "(unnamed)" : agent.GetName();
    }

    std::string ComponentLabel(const Component& comp) {
        const std::string name = comp.GetName();
        return name.empty() ? comp.GetType() : name;
    }

    //Smallest agent under the point, later ones winning ties - a background never hides what stands on it
    Agent* PickAgent(Scene& scene, const glm::vec2& world) {
        Agent* picked = nullptr;
        float pickedArea = 0.0f;
        for (auto& agent : scene.GetAgents()) {
            glm::vec2 min, max;
            if (!agent || !AgentBounds(*agent, min, max)) continue;
            if (world.x < min.x || world.x > max.x || world.y < min.y || world.y > max.y) continue;
            const float area = (max.x - min.x) * (max.y - min.y);
            if (!picked || area <= pickedArea) {
                picked = agent.get();
                pickedArea = area;
            }
        }
        return picked;
    }

    std::string SystemLabel(SceneSystem& sys) {
        const std::string name = sys.GetName();
        return name.empty() ? sys.GetType() : name;
    }
}

SceneEditor::SceneEditor(DelusiveInstance& instance)
    : instance(instance)
{
}

#pragma region History

SceneEditor::Blocks SceneEditor::Capture(Scene& scene) const {
    Blocks blocks;
    scene.CollectBlocks(blocks);
    return blocks;
}

void SceneEditor::SceneLoaded(Scene& scene) {
    selection = {};
    editingAgent = editingComponent = UUID();
    checkPending = false;
    movingAgent = movedAgent = pressInView = false;
    deferred.clear();
    saved = Capture(scene);
    history.Reset({ saved, {} });
    started = true;
}

void SceneEditor::SceneSaved(Scene& scene) {
    Blocks now = Capture(scene);
    if (now != history.Committed().blocks) history.Commit({ now, selection });
    checkPending = false;
    saved = std::move(now);
}

void SceneEditor::SettleEdits(Scene& scene) {
    if (!checkPending || ImGui::IsAnyItemActive() || movingAgent || collidersWereDragging) return;
    checkPending = false;
    Blocks now = Capture(scene);
    if (now != history.Committed().blocks) history.Commit({ std::move(now), selection });
}

bool SceneEditor::Resolves(Scene& scene, const Selection& s) const {
    switch (s.kind) {
    case Selection::Kind::Agent: {
        Agent* agent = scene.FindAgentByUUID(s.id);
        return agent && (!s.component.IsValid() || agent->GetComponentByID(s.component));
    }
    case Selection::Kind::System:
        for (auto& sys : scene.GetSystems())
            if (sys && sys->GetID() == s.id) return true;
        return false;
    default:
        return true;
    }
}

void SceneEditor::Restore(Scene& scene, const Snapshot& snap) {
    const Selection keep = selection;
    scene.LoadFromBlocks(snap.blocks);
    //Every object is new - none has its editor mode on yet
    editingAgent = editingComponent = UUID();
    //Stay on what was selected if it still exists, else go back to where the edit was made
    if (Resolves(scene, keep)) selection = keep;
    else if (Resolves(scene, snap.selection)) selection = snap.selection;
    else selection = {};
    revealComponent = selection.component.IsValid();
}

void SceneEditor::Rebuild(Scene& scene) {
    const Blocks blocks = Capture(scene);
    scene.LoadFromBlocks(blocks);
    editingAgent = editingComponent = UUID();
}

void SceneEditor::Undo(Scene& scene) {
    if (movingAgent || collidersWereDragging) return;
    //An edit that has not settled yet is still a step of its own
    Blocks now = Capture(scene);
    if (now != history.Committed().blocks) history.Commit({ now, selection });
    checkPending = false;
    if (auto snap = history.Undo({ std::move(now), selection })) Restore(scene, *snap);
}

void SceneEditor::Redo(Scene& scene) {
    if (movingAgent || collidersWereDragging) return;
    Blocks now = Capture(scene);
    if (now != history.Committed().blocks) {
        //Editing after an undo starts a new branch - there is nothing left to redo
        history.Commit({ std::move(now), selection });
        checkPending = false;
        return;
    }
    checkPending = false;
    if (auto snap = history.Redo({ std::move(now), selection })) Restore(scene, *snap);
}

#pragma endregion

#pragma region Selection

Agent* SceneEditor::SelectedAgent(Scene& scene) const {
    if (selection.kind != Selection::Kind::Agent) return nullptr;
    return scene.FindAgentByUUID(selection.id);
}

Component* SceneEditor::SelectedComponent(Scene& scene) const {
    Agent* agent = SelectedAgent(scene);
    if (!agent || !selection.component.IsValid()) return nullptr;
    return agent->GetComponentByID(selection.component);
}

SceneSystem* SceneEditor::SelectedSystem(Scene& scene) const {
    if (selection.kind != Selection::Kind::System) return nullptr;
    for (auto& sys : scene.GetSystems())
        if (sys && sys->GetID() == selection.id) return sys.get();
    return nullptr;
}

void SceneEditor::SelectAgent(const UUID& agent, const UUID& component) {
    selection.kind = Selection::Kind::Agent;
    selection.id = agent;
    selection.component = component;
    revealComponent = component.IsValid();
}

void SceneEditor::SelectSystem(const UUID& system) {
    selection.kind = Selection::Kind::System;
    selection.id = system;
    selection.component = UUID();
}

void SceneEditor::ClearSelection(Scene& scene) {
    selection = {};
    SyncEditorMode(scene);
}

void SceneEditor::SyncEditorMode(Scene& scene) {
    UUID wantAgent, wantComponent;
    if (SelectedComponent(scene)) {
        wantAgent = selection.id;
        wantComponent = selection.component;
    }
    if (wantAgent == editingAgent && wantComponent == editingComponent) return;

    if (Agent* old = scene.FindAgentByUUID(editingAgent))
        if (Component* comp = old->GetComponentByID(editingComponent)) comp->SetEditorMode(false);
    if (Component* comp = SelectedComponent(scene)) comp->SetEditorMode(true);
    editingAgent = wantAgent;
    editingComponent = wantComponent;
}

#pragma endregion

void SceneEditor::Render(Scene& scene, bool playing) {
    //The scene the editor starts on was never opened through the asset list
    if (!started && !playing) SceneLoaded(scene);
    if (!Resolves(scene, selection)) selection = {};
    SyncEditorMode(scene);

    if (!playing) {
        ViewMouse(scene);
        Shortcuts(scene);
    }
    DrawSelectionOutline(scene);

    HierarchyPanel(scene, playing);
    InspectorPanel(scene, playing);
    //Both panels had their chance to reveal it; a deferred edit below may ask again
    revealComponent = false;

    //Structural edits run once nothing is drawing the lists they change
    auto pending = std::move(deferred);
    deferred.clear();
    for (auto& edit : pending) edit(scene);
    if (!pending.empty()) SyncEditorMode(scene);

    //The play copy is thrown away on Stop - nothing in it is recorded
    if (playing) {
        checkPending = false;
        itemWasActive = false;
        return;
    }
    const bool itemActive = ImGui::IsAnyItemActive();
    if (itemWasActive && !itemActive) Touch();
    itemWasActive = itemActive;
    SettleEdits(scene);
}

#pragma region View

glm::vec2 SceneEditor::ViewCentre() const {
    return editorCamera ? editorCamera->GetPan() : glm::vec2(0.0f);
}

void SceneEditor::ViewMouse(Scene& scene) {
    const ImGuiIO& io = ImGui::GetIO();
    const glm::vec2 world = MouseWorld(instance);

    //Only presses that start on the view belong to it; ImGui keeps the rest
    if (io.MouseClicked[ImGuiMouseButton_Left]) {
        pressInView = !io.WantCaptureMouse;
        pressScreen = io.MousePos;
    }
    const bool down = io.MouseDown[ImGuiMouseButton_Left] && pressInView;
    const bool pressed = down && io.MouseClicked[ImGuiMouseButton_Left];

    //Handles of the selected collider win over picking whatever sits under them
    bool collidersDragging = false;
    if (auto* collider = dynamic_cast<ColliderComponent*>(SelectedComponent(scene))) {
        collider->HandleMouse(world, down);
        collidersDragging = collider->IsDraggingHandle();
    }
    if (collidersWereDragging && !collidersDragging) Touch();
    collidersWereDragging = collidersDragging;
    if (collidersDragging) return;

    if (pressed) {
        if (Agent* picked = PickAgent(scene, world)) {
            //Keep a selected component when clicking its own agent
            if (!(selection.kind == Selection::Kind::Agent && selection.id == picked->GetID()))
                SelectAgent(picked->GetID());
            grabOffset = picked->GetTransform().position - world;
            movingAgent = true;
            movedAgent = false;
        }
        else {
            selection = {};
        }
        return;
    }

    if (movingAgent) {
        Agent* agent = SelectedAgent(scene);
        if (!down || !agent) {
            if (movedAgent) Touch();
            movingAgent = movedAgent = false;
            return;
        }
        //A click alone never nudges - the move starts past a few pixels
        const float dx = io.MousePos.x - pressScreen.x;
        const float dy = io.MousePos.y - pressScreen.y;
        if (!movedAgent && dx * dx + dy * dy < 16.0f) return;
        movedAgent = true;
        agent->SetPosition(world + grabOffset);
    }
}

void SceneEditor::DrawSelectionOutline(Scene& scene) {
    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    const glm::mat4& projection = instance.renderer.GetProjection();
    auto Outline = [&](Agent& agent, ImU32 color, float thickness) {
        glm::vec2 min, max;
        if (!AgentBounds(agent, min, max)) return;
        const ImVec2 a = WorldToScreen({ min.x, max.y }, projection);
        const ImVec2 b = WorldToScreen({ max.x, min.y }, projection);
        draw->AddRect(a, b, color, 0.0f, 0, thickness);
    };

    //What a click would pick, while the cursor is over the view
    const ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse && !movingAgent) {
        Agent* hovered = PickAgent(scene, MouseWorld(instance));
        if (hovered && hovered != SelectedAgent(scene)) Outline(*hovered, IM_COL32(255, 255, 255, 90), 1.0f);
    }

    if (Agent* agent = SelectedAgent(scene)) Outline(*agent, IM_COL32(80, 170, 255, 255), 2.0f);
}

void SceneEditor::Shortcuts(Scene& scene) {
    const ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput || ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId)) return;

    if (ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
        if (SelectedComponent(scene)) RemoveComponent(selection.id, selection.component);
        else if (SelectedAgent(scene)) DeleteAgent(selection.id);
        else if (SelectedSystem(scene)) RemoveSystem(selection.id);
    }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_D) && SelectedAgent(scene)) DuplicateAgent(selection.id);
    if (ImGui::IsKeyPressed(ImGuiKey_F, false) && SelectedAgent(scene)) Focus(scene, selection.id);
}

void SceneEditor::Focus(Scene& scene, const UUID& agentID) {
    Agent* agent = scene.FindAgentByUUID(agentID);
    if (!agent || !editorCamera) return;
    glm::vec2 min, max;
    AgentBounds(*agent, min, max);
    editorCamera->SetPan((min + max) * 0.5f);
}

#pragma endregion

#pragma region Structural edits

void SceneEditor::AddAgent(const std::string& type) {
    deferred.push_back([this, type](Scene& scene) {
        std::unique_ptr<Agent> agent = DelusiveFactory<Agent>::Create(type, instance);
        if (!agent) return;
        agent->SetID(UUID::GenerateRandom());
        agent->SetName(type);
        agent->SetPosition(ViewCentre());
        const UUID id = agent->GetID();
        scene.AddAgent(std::move(agent));
        SelectAgent(id);
        Touch();
    });
}

void SceneEditor::AddPrefab(const std::string& path) {
    deferred.push_back([this, path](Scene& scene) {
        std::unique_ptr<Agent> source = Agent::LoadFromFile(path, instance, &scene);
        if (!source) return;
        //Fresh ids - the placed copy belongs to the scene, the file keeps its own
        std::unique_ptr<Agent> placed = DelusiveInstantiate<Agent>(*source, instance,
            [&scene](Agent& a) { a.LinkScene(&scene); });
        if (!placed) return;
        placed->SetPosition(ViewCentre());
        const UUID id = placed->GetID();
        scene.AddAgent(std::move(placed));
        SelectAgent(id);
        Touch();
    });
}

void SceneEditor::AddSystem(const std::string& type) {
    deferred.push_back([this, type](Scene& scene) {
        std::unique_ptr<SceneSystem> sys = DelusiveFactory<SceneSystem>::Create(type, instance);
        if (!sys) return;
        sys->SetID(UUID::GenerateRandom());
        const UUID id = sys->GetID();
        scene.AddSystem(std::move(sys));
        SelectSystem(id);
        Touch();
    });
}

void SceneEditor::DuplicateAgent(const UUID& agentID) {
    deferred.push_back([this, agentID](Scene& scene) {
        Agent* source = scene.FindAgentByUUID(agentID);
        if (!source) return;
        std::unique_ptr<Agent> copy = DelusiveInstantiate<Agent>(*source, instance,
            [&scene](Agent& a) { a.LinkScene(&scene); });
        if (!copy) return;
        const UUID id = copy->GetID();
        scene.AddAgent(std::move(copy));
        SelectAgent(id);
        Touch();
    });
}

void SceneEditor::DeleteAgent(const UUID& agentID) {
    deferred.push_back([this, agentID](Scene& scene) {
        if (!scene.RemoveAgent(agentID)) return;
        if (selection.kind == Selection::Kind::Agent && selection.id == agentID) selection = {};
        //Links to it (a camera's follow target) are resolved again, to nothing
        Rebuild(scene);
        Touch();
    });
}

void SceneEditor::RemoveComponent(const UUID& agentID, const UUID& componentID) {
    deferred.push_back([this, agentID, componentID](Scene& scene) {
        Agent* agent = scene.FindAgentByUUID(agentID);
        Component* comp = agent ? agent->GetComponentByID(componentID) : nullptr;
        if (!comp) return;
        if (editingComponent == componentID) editingAgent = editingComponent = UUID();
        agent->RemoveComponentByPointer(comp);
        if (selection.component == componentID) selection.component = UUID();
        //Siblings holding it (an animator's sprite) let go of it
        Rebuild(scene);
        Touch();
    });
}

void SceneEditor::MoveComponent(const UUID& agentID, const UUID& componentID, int delta) {
    deferred.push_back([this, agentID, componentID, delta](Scene& scene) {
        Agent* agent = scene.FindAgentByUUID(agentID);
        Component* comp = agent ? agent->GetComponentByID(componentID) : nullptr;
        if (!comp) return;
        agent->MoveComponent(comp, delta);
        Touch();
    });
}

void SceneEditor::AddComponent(const UUID& agentID, const std::string& type) {
    deferred.push_back([this, agentID, type](Scene& scene) {
        Agent* agent = scene.FindAgentByUUID(agentID);
        if (!agent) return;
        std::unique_ptr<Component> comp = DelusiveComponentFactory::CreateComponentByType(type, instance);
        if (!comp) return;
        comp->SetID(UUID::GenerateRandom());
        const UUID id = comp->GetID();
        agent->AddRawComponent(std::move(comp));
        SelectAgent(agentID, id);
        Touch();
    });
}

void SceneEditor::RemoveSystem(const UUID& systemID) {
    deferred.push_back([this, systemID](Scene& scene) {
        auto& systems = scene.GetSystems();
        auto found = std::find_if(systems.begin(), systems.end(),
            [&](const std::unique_ptr<SceneSystem>& s) { return s && s->GetID() == systemID; });
        if (found == systems.end()) return;
        systems.erase(found);
        if (selection.kind == Selection::Kind::System && selection.id == systemID) selection = {};
        Rebuild(scene);
        Touch();
    });
}

#pragma endregion

#pragma region Hierarchy

void SceneEditor::AddMenu(Scene& scene) {
    if (!ImGui::BeginPopup("AddToScene")) return;

    if (ImGui::BeginMenu("Agent")) {
        for (const std::string& type : DelusiveFactory<Agent>::Types())
            if (ImGui::MenuItem(type.c_str())) AddAgent(type);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("System")) {
        for (const std::string& type : DelusiveFactory<SceneSystem>::Types()) {
            bool present = false;
            for (auto& sys : scene.GetSystems()) present |= sys && sys->GetType() == type;
            if (ImGui::MenuItem(type.c_str(), present ? "in scene" : nullptr, false, !present)) AddSystem(type);
        }
        ImGui::EndMenu();
    }

    //Agent files placed as independent copies
    if (ImGui::BeginMenu("Prefab")) {
        std::vector<std::filesystem::path> files;
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(AGENT_PATH, ec))
            if (entry.is_regular_file() && entry.path().extension() == AGENT_EXT) files.push_back(entry.path());
        std::sort(files.begin(), files.end());
        if (files.empty()) ImGui::TextDisabled("No agent files in %s", AGENT_PATH);
        for (const auto& file : files)
            if (ImGui::MenuItem(file.stem().string().c_str())) AddPrefab(file.string());
        ImGui::EndMenu();
    }

    ImGui::EndPopup();
}

void SceneEditor::HierarchyPanel(Scene& scene, bool playing) {
    if (!ImGui::Begin(HierarchyWindow)) {
        ImGui::End();
        return;
    }

    //Scene name - the buffer follows the scene except while it is being typed in
    if (!editingSceneName) std::snprintf(sceneName, sizeof(sceneName), "%s", scene.GetName().c_str());
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputTextWithHint("##SceneName", "Scene name", sceneName, sizeof(sceneName)))
        scene.SetName(sceneName);
    editingSceneName = ImGui::IsItemActive();

    ImGui::BeginDisabled(playing);
    if (ImGui::Button("+ Add")) ImGui::OpenPopup("AddToScene");
    ImGui::EndDisabled();
    AddMenu(scene);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputTextWithHint("##Filter", "Filter", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf)))
        filter.Build();
    ImGui::Separator();

    ImGui::BeginChild("Tree");

    //Systems
    auto& systems = scene.GetSystems();
    char header[64];
    std::snprintf(header, sizeof(header), "Systems (%d)###Systems", (int)systems.size());
    if (ImGui::TreeNodeEx(header, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth)) {
        for (auto& sys : systems) {
            if (!sys) continue;
            const std::string label = SystemLabel(*sys);
            if (!filter.PassFilter(label.c_str())) continue;

            ImGui::PushID(sys->GetID().ToString().c_str());
            const bool isSelected = selection.kind == Selection::Kind::System && selection.id == sys->GetID();
            if (ImGui::Selectable(label.c_str(), isSelected))
                SelectSystem(sys->GetID());
            if (ImGui::BeginPopupContextItem("SystemMenu")) {
                if (ImGui::MenuItem("Remove", "Del", false, !playing)) RemoveSystem(sys->GetID());
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    //Agents, each opening into its components
    auto& agents = scene.GetAgents();
    std::snprintf(header, sizeof(header), "Agents (%d)###Agents", (int)agents.size());
    if (ImGui::TreeNodeEx(header, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth)) {
        for (auto& owned : agents) {
            if (!owned) continue;
            Agent& agent = *owned;
            const UUID agentID = agent.GetID();
            const std::string label = AgentLabel(agent);

            bool passes = filter.PassFilter(label.c_str());
            for (const auto& comp : agent.GetComponents())
                passes |= comp && filter.PassFilter(ComponentLabel(*comp).c_str());
            if (!passes) continue;

            ImGui::PushID(agentID.ToString().c_str());
            const bool agentSelected = selection.kind == Selection::Kind::Agent && selection.id == agentID;
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (agentSelected && !selection.component.IsValid()) flags |= ImGuiTreeNodeFlags_Selected;
            if (agent.GetComponents().empty()) flags |= ImGuiTreeNodeFlags_Leaf;
            //Selecting a component reveals it
            if (agentSelected && selection.component.IsValid() && revealComponent) ImGui::SetNextItemOpen(true);

            const bool open = ImGui::TreeNodeEx("##agent", flags, "%s", label.c_str());
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen()) SelectAgent(agentID);
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) Focus(scene, agentID);

            //Dropped on link fields in the inspector
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("DND_AGENT_UUID", &agentID, sizeof(UUID));
                ImGui::Text("%s", label.c_str());
                ImGui::EndDragDropSource();
            }
            if (ImGui::BeginPopupContextItem("AgentMenu")) {
                if (ImGui::MenuItem("Focus", "F")) Focus(scene, agentID);
                if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, !playing)) DuplicateAgent(agentID);
                if (ImGui::MenuItem("Delete", "Del", false, !playing)) DeleteAgent(agentID);
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            ImGui::TextDisabled("%s", agent.GetType().c_str());

            if (open) {
                const auto& comps = agent.GetComponents();
                for (size_t i = 0; i < comps.size(); ++i) {
                    Component* comp = comps[i].get();
                    if (!comp) continue;
                    const UUID compID = comp->GetID();
                    ImGui::PushID(compID.ToString().c_str());
                    const bool compSelected = agentSelected && selection.component == compID;
                    if (ImGui::Selectable(ComponentLabel(*comp).c_str(), compSelected))
                        SelectAgent(agentID, compID);
                    if (ImGui::BeginPopupContextItem("ComponentMenu")) {
                        if (ImGui::MenuItem("Move up", nullptr, false, !playing && i > 0)) MoveComponent(agentID, compID, -1);
                        if (ImGui::MenuItem("Move down", nullptr, false, !playing && i + 1 < comps.size())) MoveComponent(agentID, compID, 1);
                        if (ImGui::MenuItem("Remove", "Del", false, !playing)) RemoveComponent(agentID, compID);
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }

    ImGui::EndChild();
    ImGui::End();
}

#pragma endregion

#pragma region Inspector

void SceneEditor::InspectorPanel(Scene& scene, bool playing) {
    if (!ImGui::Begin(InspectorWindow)) {
        ImGui::End();
        return;
    }
    if (playing) ImGui::TextDisabled("Playing - changes made here are not kept");

    //Labels sit to the right of their inputs, so leave them room
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);

    if (SceneSystem* sys = SelectedSystem(scene)) {
        ImGui::Text("%s", SystemLabel(*sys).c_str());
        ImGui::TextDisabled("%s", sys->GetType().c_str());
        ImGui::Separator();
        sys->DrawImGui();
    }
    else if (Agent* agent = SelectedAgent(scene)) {
        const UUID agentID = agent->GetID();
        ImGui::TextDisabled("%s", agent->GetType().c_str());
        agent->DrawPropertiesImGui();

        ImGui::SeparatorText("Components");
        const auto& comps = agent->GetComponents();
        for (size_t i = 0; i < comps.size(); ++i) {
            Component* comp = comps[i].get();
            if (!comp) continue;
            const UUID compID = comp->GetID();
            const bool compSelected = selection.component == compID;
            ImGui::PushID(compID.ToString().c_str());

            if (compSelected && revealComponent) ImGui::SetNextItemOpen(true);
            if (compSelected) ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_HeaderActive));
            const float right = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
            const std::string label = ComponentLabel(*comp) + "###header";
            const bool open = ImGui::CollapsingHeader(label.c_str(),
                ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
            if (compSelected) ImGui::PopStyleColor();
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) SelectAgent(agentID, compID);
            if (compSelected && revealComponent) {
                ImGui::SetScrollHereY(0.0f);
                revealComponent = false;
            }

            //Per-component menu at the right end of its header
            const float menuWidth = ImGui::CalcTextSize("...").x + ImGui::GetStyle().FramePadding.x * 2.0f;
            ImGui::SameLine(right - menuWidth);
            if (ImGui::SmallButton("...")) ImGui::OpenPopup("ComponentMenu");
            if (ImGui::BeginPopup("ComponentMenu")) {
                if (ImGui::MenuItem("Move up", nullptr, false, !playing && i > 0)) MoveComponent(agentID, compID, -1);
                if (ImGui::MenuItem("Move down", nullptr, false, !playing && i + 1 < comps.size())) MoveComponent(agentID, compID, 1);
                if (ImGui::MenuItem("Remove", nullptr, false, !playing)) RemoveComponent(agentID, compID);
                ImGui::EndPopup();
            }

            if (open) {
                ImGui::Indent();
                comp->DrawImGui();
                ImGui::Unindent();
                ImGui::Spacing();
            }
            ImGui::PopID();
        }
        revealComponent = false;

        ImGui::Spacing();
        ImGui::BeginDisabled(playing);
        if (ImGui::Button("Add component", ImVec2(-FLT_MIN, 0.0f))) ImGui::OpenPopup("AddComponent");
        ImGui::EndDisabled();
        if (ImGui::BeginPopup("AddComponent")) {
            const std::string type = DelusiveComponentFactory::DrawComponentAddMenu();
            if (!type.empty()) {
                AddComponent(agentID, type);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    else {
        ImGui::TextDisabled("Select an agent or system - in the hierarchy, or click it in the view.");
    }

    ImGui::PopItemWidth();
    ImGui::End();
}

#pragma endregion

void SceneEditor::RenderStatusBar(Scene& scene, bool playing) {
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    if (ImGui::BeginViewportSideBar("##SceneStatus", ImGui::GetMainViewport(), ImGuiDir_Down, ImGui::GetFrameHeight(), flags)) {
        if (ImGui::BeginMenuBar()) {
            if (playing) ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "Playing");
            else if (IsDirty()) ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.3f, 1.0f), "Unsaved changes");
            else ImGui::TextDisabled("Saved");
            ImGui::Separator();
            ImGui::Text("%d agents, %d systems", (int)scene.GetAgents().size(), (int)scene.GetSystems().size());
            ImGui::Separator();
            const glm::vec2 world = MouseWorld(instance);
            ImGui::Text("Cursor %.2f, %.2f", world.x, world.y);
            ImGui::EndMenuBar();
        }
    }
    ImGui::End();
}
