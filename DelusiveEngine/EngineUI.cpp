#include <filesystem>
#include "EngineUI.h"
#include "DelusiveAgents.h"
#include "DelusiveMacros.h"
#include "DelusiveRenderer.h"
#include "DelusiveComponents.h"
#include "DelusiveUtils.h"
#include "DelusiveSystems.h"
#include <glm/gtc/type_ptr.hpp>

EngineUI::EngineUI(GameManager& _game, DelusiveRenderer& _renderer)
    : gameManager(_game), renderer(_renderer)
{
    currentMode = EditorMode::SceneEditor;
    loadedAssets = LoadSceneList();
}

EngineUI::~EngineUI() {
    for (auto& branch : currentAnimation.data.branches) {
        for (auto& frame : branch.frames) {
            if (frame.previewTexture != 0) {
                glDeleteTextures(1, &frame.previewTexture);
                frame.previewTexture = 0;
            }
        }
    }
}

const char* ViewModeToString(EditorMode mode) {
    switch (mode) {
    case EditorMode::SceneEditor: return "Scene Editor";
    case EditorMode::AgentEditor: return "Agent Editor";
    case EditorMode::AnimatorEditor: return "Animator";
    case EditorMode::GameView: return "Game View";
    }
    return "Unknown";
}

std::vector<std::string> EngineUI::LoadSceneList() {
    std::vector<std::string> sceneNames;

    std::string path;
    switch (currentMode) {
    case EditorMode::SceneEditor:
        path = SCENE_PATH;
        break;
    case EditorMode::AgentEditor:
        path = AGENT_PATH;
        break;
    case EditorMode::AnimatorEditor:
        path = ANIM_PATH;
        break;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().stem().string();
            //filter by extension code
            if (entry.path().extension() == ".scene" || entry.path().extension() == ".agent" || entry.path().extension() == ".anim") {
                sceneNames.push_back(filename);
            }
        }
    }

    return sceneNames;
}

ImTextureID EngineUI::GetFramePreviewTexture(AnimationFrame& frame, Agent& baseAgent) {
    if (!frame.dirty && frame.previewTexture != 0)
        return (ImTextureID)(intptr_t)frame.previewTexture;

    std::unique_ptr<Agent> tempAgent = baseAgent.Clone();
    ApplyOverrides(frame, *tempAgent);

    if (frame.previewTexture != 0)
        glDeleteTextures(1, &frame.previewTexture);

    frame.previewTexture = tempAgent->RenderAgentToTexture(256, 256);
    frame.dirty = false;

    return (ImTextureID)(intptr_t)frame.previewTexture;
}

void EngineUI::Render(Scene& scene) {
    RenderTopBar(scene);

    switch (currentMode) {
    case EditorMode::SceneEditor:
        RenderSceneEditor(scene);
        break;
    case EditorMode::AgentEditor:
        RenderAgentEditor(scene);
        break;
    case EditorMode::AnimatorEditor:
        RenderAnimatorEditor(scene);
        break;
    case EditorMode::GameView:
        RenderGameView(scene);
        break;
    }
}

void EngineUI::SwitchMode(Scene& scene, EditorMode mode) {
    currentMode = mode;
    selectedAsset = "None";

    selectedComponent = nullptr;
    selectedFrame = -1;
    selectedBranch = -1;
    ClearFramePreviews(currentAnimation);
    currentAnimation.Clear();
    baseAgent.reset();
    pureAgent.reset();

    scene.Clear();

    switch (mode) {
    case EditorMode::SceneEditor:
        scene.SetName("Scene Editor");
        break;
    case EditorMode::AgentEditor:
        scene.SetName("Agent Editor");
        break;
    case EditorMode::AnimatorEditor:
        scene.SetName("Animator");
        break;
    default:
        scene.SetName("New Scene");
    }

    loadedAssets = LoadSceneList();
}

std::string EngineUI::GetPath(std::string fileName) {
    std::string fullPath = "";
    switch (currentMode) {
    case EditorMode::SceneEditor:
        return std::string(SCENE_PATH) + fileName + SCENE_EXT;
    case EditorMode::AgentEditor:
        return std::string(AGENT_PATH) + fileName + AGENT_EXT;
    case EditorMode::AnimatorEditor:
        return std::string(ANIM_PATH) + fileName + ANIM_EXT;
    default:
        return "";
    }
}

void EngineUI::RenderTopBar(Scene& scene) {
    if (ImGui::BeginMainMenuBar()) {
        static bool newAssetPopup = false;
        static std::string assetToDelete;
        static bool showDeleteConfirm = false;

        ImGui::Text("View");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(175.0f);
        if (ImGui::BeginCombo("##ViewModeSelector", ViewModeToString(currentMode))) {
            if (ImGui::Selectable("Scene Editor", currentMode == EditorMode::SceneEditor)) {
                SwitchMode(scene, EditorMode::SceneEditor);    
            }
            if (ImGui::Selectable("Agent Editor", currentMode == EditorMode::AgentEditor)) {
                SwitchMode(scene, EditorMode::AgentEditor);
            }
            if (ImGui::Selectable("Animator", currentMode == EditorMode::AnimatorEditor)) {
                SwitchMode(scene, EditorMode::AnimatorEditor);
            }
            if (ImGui::Selectable("Game View", currentMode == EditorMode::GameView)) {
                currentMode = EditorMode::GameView;
            }
            if (gameManager.IsPlaying()) {
                gameManager.Stop();
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine(0.0f, 10.0f);
        const char* assetLabel = (currentMode == EditorMode::SceneEditor) ? "Loaded Scene: " : "Loaded Asset: ";
        ImGui::Text("%s", assetLabel);

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SameLine();
        if (ImGui::BeginCombo("##AssetSelector", selectedAsset.c_str())) {
            if (ImGui::Selectable("Add New...")) {
                newAssetPopup = true;
            }

            for (const auto& asset : loadedAssets) {
                bool isSelected = (selectedAsset == asset);
                ImGui::PushID(asset.c_str());
                if (ImGui::Selectable(asset.c_str(), isSelected)) {
                    selectedAsset = asset;
                    std::string fullPath = GetPath(asset);
                    switch (currentMode) {
                    case EditorMode::SceneEditor: {
                        scene.LoadFromFile(fullPath);
                        break;
                    }
                    case EditorMode::AgentEditor: {
                        scene.ClearAgents();
                        selectedComponent = nullptr;

                        std::ifstream in(fullPath);
                        if (in.is_open()) {
                            auto agent = std::make_unique<PlayerAgent>("Temp");
                            agent->LoadFromFile(in);
                            scene.AddAgent(std::move(agent));
                        }
                        break;
                    }
                    case EditorMode::AnimatorEditor: {
                        ClearFramePreviews(currentAnimation);

                        currentAnimation.Clear();
                        currentAnimation.LoadFromFile(fullPath);

                        SetupAnimation(currentAnimation.data.defaultAgentPath);
                        break;
                    }
                    default: break;
                    }
                }
                if (ImGui::BeginPopupContextItem("SceneContextMenu")) {
                    if (ImGui::MenuItem("Delete")) {
                        assetToDelete = asset;
                        showDeleteConfirm = true;
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            std::string path;
            std::string name = selectedAsset.empty() ? "Unnamed" : selectedAsset;

            std::string savePath = GetPath(selectedAsset);
            switch (currentMode) {
            case EditorMode::SceneEditor: {
                scene.SaveToFile(savePath);
                break;
            }
            case EditorMode::AgentEditor: {
                if (!scene.GetAgents().empty()) {
                    std::ofstream out(savePath);
                    if (out.is_open()) {
                        scene.GetAgents().front()->SaveToFile(out);  // Save the first (and only) agent
                        out.close();
                    }
                }
                break;
            }
            case EditorMode::AnimatorEditor: {
                currentAnimation.SaveToFile(savePath);
                break;
            }
            }
            loadedAssets = LoadSceneList();
        }

        ImGui::SameLine();
        if (ImGui::Button("Delete")) {
            showDeleteConfirm = true;
            assetToDelete = selectedAsset;
        }

        ImGui::SameLine();
        if (currentMode == EditorMode::SceneEditor || currentMode == EditorMode::GameView) {
            if (ImGui::Button(gameManager.IsPlaying() ? "Stop" : "Play")) {
                if (gameManager.IsPlaying()) {
                    gameManager.Stop();
                }
                else {
                    gameManager.Play();
                }
            }
        }

        ImGui::EndMainMenuBar();

        if (showDeleteConfirm) {
            ImGui::OpenPopup("Confirm Delete Scene");
        }

        if (ImGui::BeginPopupModal("Confirm Delete Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure you want to delete: %s", assetToDelete.c_str());

            if (ImGui::Button("Yes, Delete")) {
                std::string fullPath = GetPath(assetToDelete);

                std::remove(fullPath.c_str());
                loadedAssets = LoadSceneList();
                if (selectedAsset == assetToDelete) selectedAsset = "";
                assetToDelete.clear();
                showDeleteConfirm = false;
                ImGui::CloseCurrentPopup();
                loadedAssets = LoadSceneList();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                assetToDelete.clear();
                showDeleteConfirm = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (newAssetPopup) {
            ImGui::OpenPopup("NewAssetPopup");
            newAssetPopup = false;
        }

        if (ImGui::BeginPopup("NewAssetPopup")) {
            static char assetNameBuffer[64] = "";

            ImGui::InputText("Name", assetNameBuffer, sizeof(assetNameBuffer));
            if (ImGui::Button("Create")) {
                if (strlen(assetNameBuffer) > 0) {
                    loadedAssets.push_back(assetNameBuffer);
                    selectedAsset = assetNameBuffer;
                    
                    std::string sceneName = "New Asset";
                    switch (currentMode) {
                    case EditorMode::SceneEditor:
                        sceneName = assetNameBuffer;
                        break;
                    case EditorMode::AgentEditor:
                        sceneName = "Agent Editing View";
                        break;
                    case EditorMode::AnimatorEditor:
                        sceneName = "Animator";
                        break;
                    }

                    scene.Clear();
                    scene.SetName(sceneName);

                    std::ofstream out(GetPath(selectedAsset));
                    out << "name " << selectedAsset << "\n";
                    out.close();

                    assetNameBuffer[0] = '\0';
                    loadedAssets = LoadSceneList();
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}

void EngineUI::RenderSceneEditor(Scene& scene) {
    struct Selection {
        enum Kind { None = 0, AgentObject = 1, ComponentObject = 2, SystemObject = 3 } kind = None;
		void* ptr = nullptr;
		void Reset() { kind = None; ptr = nullptr; }
		bool Is(Kind k, void* p) const { return kind == k && ptr == p; }
        void Draw() {
            // Use selection.kind to safely cast to the correct type
            switch (kind) {
            case AgentObject:
                if (ptr) {
                    Agent* a = static_cast<Agent*>(ptr);
                    a->DrawImGui();
                }
                break;
            case ComponentObject:
                if (ptr) {
                    Component* c = static_cast<Component*>(ptr);
                    c->DrawImGui();
                }
                break;
            case SystemObject:
                if (ptr) {
                    SceneSystem* s = static_cast<SceneSystem*>(ptr);
                    s->DrawImGui();
                }
                break;
            default:
                ImGui::TextDisabled("Nothing selected.");
                break;
            }   
        }
        void SetEditorMode(bool enabled) {
            switch (kind) {
            case AgentObject:
                if (ptr) {
                    Agent* a = static_cast<Agent*>(ptr);
                    a->SetEditorMode(enabled);
                }
                break;
            case ComponentObject:
                if (ptr) {
                    Component* c = static_cast<Component*>(ptr);
                    c->SetEditorMode(enabled);
                }
                break;
            case SystemObject:
                if (ptr) {
                    SceneSystem* s = static_cast<SceneSystem*>(ptr);
                    s->SetEditorMode(enabled);
                }
                break;
            default:
                ImGui::TextDisabled("Nothing selected.");
                break;
            }
        }
    };
    static Selection selected;

    float topBarHeight = ImGui::GetFrameHeight();
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, topBarHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.3f, ImGui::GetIO().DisplaySize.y - topBarHeight), ImGuiCond_Always);
    ImGui::Begin("Scene Editor Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    if (ImGui::BeginChild("Hierarchy", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), true)) {
        ImGui::Text("%s Hierarchy", scene.GetName().c_str());
        if (ImGui::BeginPopupContextWindow("SceneRightClick", ImGuiPopupFlags_MouseButtonRight)) {
            if (ImGui::BeginMenu("Add Agent")) {
                if (ImGui::MenuItem("Player Agent")) {
                    scene.AddAgent(std::make_unique<PlayerAgent>("NewPlayer"));
                }
                if (ImGui::MenuItem("Camera Agent")) {
                    scene.AddAgent(std::make_unique<CameraAgent>());
                }
                if (ImGui::MenuItem("Enemy Agent")) {
                    scene.AddAgent(std::make_unique<EnemyAgent>("NewEnemy"));
                }
                if (ImGui::MenuItem("Environment Agent")) {
                    scene.AddAgent(std::make_unique<EnvironmentAgent>("NewEnvironment"));
                }
                ImGui::EndMenu();
            }

            // Future: Add other right-click tools like "Paste", "Add Empty", "Create Group", etc.

            ImGui::EndPopup();
        }

        ImGui::Indent();
        if (ImGui::TreeNode("Systems")) {
            auto& systems = scene.GetSystems();

            // List systems like agents
            for (size_t i = 0; i < systems.size(); ++i) {
                auto& system = systems[i];
                std::string label = system->GetName().empty()
                    ? "System " + std::to_string(i)
                    : system->GetName();

                ImGui::PushID((int)i);

                // Check if this system is the selected one
                bool isSelected = (selected.kind == Selection::SystemObject &&
                    selected.ptr == system.get());

                // System selectable
                if (ImGui::Selectable(label.c_str(), isSelected)) {
                    if (selected.ptr) selected.SetEditorMode(false); // deselect previous
                    selected.kind = Selection::SystemObject;
                    selected.ptr = system.get();
                    selected.SetEditorMode(true); // select new
                }

                // Right-click popup for delete and future options
                if (ImGui::BeginPopupContextItem("SystemContextMenu", ImGuiPopupFlags_MouseButtonRight)) {
                    if (ImGui::MenuItem("Delete")) {
                        // If deleting the currently selected system, clear selection
                        if (selected.ptr == system.get()) {
                            selected.SetEditorMode(false);
                            selected.Reset();
                        }

                        systems.erase(systems.begin() + i);

                        ImGui::EndPopup();
                        ImGui::PopID();
                        break; // exit loop, iterator invalid now
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopID();
            }

            // Add system popup menu (triggered by right-clicking background of Systems tree node)
            if (ImGui::BeginPopupContextItem("AddSystemPopup", ImGuiPopupFlags_MouseButtonRight)) {
                if (ImGui::BeginMenu("Add System")) {
                    if (ImGui::MenuItem("PathfindingSystem")) {
                        scene.AddSystem(std::make_unique<PathfindingSystem>(renderer));
                    }
                    if (ImGui::MenuItem("UIManager")) {
                        scene.AddSystem(std::make_unique<UIManager>(renderer));
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Agents")) {
            auto& agents = scene.GetAgents();
            for (size_t i = 0; i < agents.size(); ++i) {
				Agent* agent = agents[i].get();
                std::string agentName = agent->GetName().empty() ? "Agent " + std::to_string(i) : agent->GetName();

                ImGui::PushID((int)i);

                ImGuiTreeNodeFlags flags =
                    ImGuiTreeNodeFlags_OpenOnArrow |
                    ImGuiTreeNodeFlags_OpenOnDoubleClick |
                    ImGuiTreeNodeFlags_SpanAvailWidth |
                    (selected.Is(Selection::AgentObject, agent) ? ImGuiTreeNodeFlags_Selected : 0);

                bool agentOpen = ImGui::TreeNodeEx(agentName.c_str(), flags);
                if (ImGui::IsItemClicked()) {
					selected.SetEditorMode(false); // Deselect previous
					selected.kind = Selection::AgentObject;
                    selected.ptr = agent;
					selected.SetEditorMode(true); // Enable editor mode for new selection
                }


                if (ImGui::BeginPopupContextItem("AgentContextMenu", ImGuiPopupFlags_MouseButtonRight)) {
                    if (ImGui::MenuItem("Delete")) {
                        agentToDeleteIndex = (int)i;
                    }
                    if (ImGui::BeginMenu("Load Prefab")) {
                        for (const auto& entry : std::filesystem::directory_iterator(AGENTS_FOLDER)) {
                            if (entry.path().extension() == ".agent") {
                                if (ImGui::MenuItem(entry.path().filename().string().c_str())) {
                                    auto loaded = std::make_unique<PlayerAgent>("LoadedAgent");
                                    loaded->LoadFromFile(entry.path().string());
                                    scene.AddAgent(std::move(loaded));
                                }
                            }
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }

                //Component list
                if (agentOpen) {
                    auto& comps = agent->GetComponents();
                    for (size_t c = 0; c < comps.size(); ++c) {
                        Component* comp = comps[c].get();
                        ImGui::PushID((int)c);

                        bool compSelected = selected.Is(Selection::ComponentObject, comp);
                        if (ImGui::Selectable(comp->GetName().c_str(), compSelected)) {
                            selected.SetEditorMode(false);
                            selected.kind = Selection::ComponentObject;
                            selected.ptr = comp;
							selected.SetEditorMode(true);
                        }

                        if (ImGui::BeginPopupContextItem(("ComponentContextMenu##" + std::to_string(i) + "_" + std::to_string(c)).c_str(), ImGuiPopupFlags_MouseButtonRight)) {
                            if (ImGui::MenuItem("Delete Component")) {
                                // deletion deferred to avoid invalidating iteration
                                agent->RemoveComponentByPointer(comp);
                                if (selected.Is(Selection::ComponentObject, comp)) selected.Reset();
                            }
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
    }
    ImGui::EndChild();

    // After the ImGui loop, handle deletion once safely
    if (agentToDeleteIndex >= 0 && agentToDeleteIndex < (int)scene.GetAgents().size()) {
        scene.GetAgents().erase(scene.GetAgents().begin() + agentToDeleteIndex);

        // Adjust selectedAgentIndex accordingly
        if(selected.ptr == scene.GetAgents()[agentToDeleteIndex].get()) {
            selected.Reset(); // Deselect if the deleted agent was selected
		}

        agentToDeleteIndex = -1; // reset delete index
    }

    ImGui::SameLine();

    if (ImGui::BeginChild("Inspector", ImVec2(0, 0), true)) {
        selected.Draw();
    }
    ImGui::EndChild();

    ImGui::End();
}

void EngineUI::RenderAgentEditor(Scene& scene) {
    //Mouse stuff
    if (!ImGui::GetIO().WantCaptureMouse) {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        glm::vec2 worldMouse = ScreenToWorld2D((int)mouseX, (int)mouseY, renderer.GetProjection());
        bool mouseDown = SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK;

        Agent& agent = *scene.GetAgents().front();
        for (auto& comp : agent.GetComponents()) {
            if (isDraggingCollider)
                continue;

            comp->HandleMouse(worldMouse, mouseDown);
        }
    }

    float topBarHeight = ImGui::GetFrameHeight();
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, topBarHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.3f, ImGui::GetIO().DisplaySize.y - topBarHeight), ImGuiCond_Always);
    ImGui::Begin("Agent Editor Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    auto& agents = scene.GetAgents();
    if (agents.empty()) {
        scene.AddAgent(std::make_unique<PlayerAgent>("NewAgent"));
    }

    Agent& agent = *scene.GetAgents().front();

    if (ImGui::BeginChild("Component List", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), true)) {
        if (ImGui::Selectable(agent.GetName().c_str(), agentSelected)) {
            if(selectedComponent) selectedComponent->SetEditorMode(false);
            selectedComponent = nullptr;
            agentSelected = true;
        } 

        if (ImGui::BeginPopupContextItem("AgentContext")) { 
            if (ImGui::BeginMenu("Add")) { 
                if (ImGui::MenuItem("Sprite")) {
                    selectedComponent = agent.AddComponent<SpriteComponent>(DEFAULT_SPRITE);
                    agentSelected = false;
                }
                if (ImGui::BeginMenu("Collider")) {
                    if (ImGui::MenuItem("Solid")) {
                        selectedComponent = agent.AddComponent<SolidCollider>();
                        agentSelected = false;
                    }
                    if (ImGui::MenuItem("Hitbox")) {
                        selectedComponent = agent.AddComponent<HitboxCollider>();
                        agentSelected = false;
                    }
                    if (ImGui::MenuItem("Hurtbox")) {
                        selectedComponent = agent.AddComponent<HurtboxCollider>();
                        agentSelected = false;
                    }
                    if (ImGui::MenuItem("Trigger")) {
                        selectedComponent = agent.AddComponent<TriggerCollider>();
                        agentSelected = false;
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Add AnimatorComponent")) {
                    selectedComponent = agent.AddComponent<AnimatorComponent>();
                    agentSelected = false;
                }
                if (ImGui::MenuItem("Stats")) {
                    selectedComponent = agent.AddComponent<StatsComponent>();
                    agentSelected = false;
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }

        int i = 0;
        ImGui::Indent();
        for (const auto& comp : agent.GetComponents()) {
            ImGui::PushID(i);
            bool isSelected = (selectedComponent == comp.get());
            if (ImGui::Selectable(comp->GetName().c_str(), isSelected)) {
				if (selectedComponent) selectedComponent->SetEditorMode(false);
                selectedComponent = comp.get();
				selectedComponent->SetEditorMode(true);
                agentSelected = false;
            }

            if (ImGui::BeginPopupContextItem("ComponentContext")) {
                if (ImGui::MenuItem("Remove")) {
                    agent.RemoveComponentByPointer(comp.get());
                    if (selectedComponent == comp.get()) {
                        selectedComponent = nullptr;
                        agentSelected = true;
                    }
                    ImGui::EndPopup();
                    ImGui::PopID();
                    break;
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();
            ++i;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    if (ImGui::BeginChild("Component Inspector", ImVec2(0, 0), true)) {
        ImGui::Text("Inspector");
        ImGui::Separator();
        ImGui::Separator();

        if (agentSelected) {
            char nameBuffer[64];
            strncpy_s(nameBuffer, agent.GetName().c_str(), sizeof(nameBuffer));
            ImGui::Text("Name");
            ImGui::SameLine();
            if (ImGui::InputText("##nameInput", nameBuffer, sizeof(nameBuffer))) {
                if (strlen(nameBuffer) > 0) {
                    agent.SetName(std::string(nameBuffer));
                }
                else {
                    // Restore or ignore empty name
                    strncpy_s(nameBuffer, agent.GetName().c_str(), sizeof(nameBuffer));
                }
            }

            static char descBuffer[256] = "";
            ImGui::Text("Description");
            ImGui::InputTextMultiline("##descInput", descBuffer, sizeof(descBuffer), ImVec2(-1, 60));


            ImGui::Text("\nTransform");
            ImGui::Separator();
            ImGui::Text("Position: ");
            ImGui::SameLine();
            glm::vec2 pos = agent.GetTransform().position;
            if (ImGui::DragFloat2("##position", glm::value_ptr(pos), 1.0f)) {
                agent.SetPosition(pos);
            }

            ImGui::Text("Rotation: ");
            ImGui::SameLine();
            float rot = agent.GetTransform().rotation;
            if (ImGui::DragFloat("##rotation", &rot, 0.1f)) {
                agent.SetRotation(rot);
            }

            ImGui::Text("Scale:    ");
            ImGui::SameLine();
            glm::vec2 scale = agent.GetTransform().scale;
            if (ImGui::DragFloat2("##scale", glm::value_ptr(scale), 0.1f)) {
                agent.SetScale(scale);
            }
        }
        else if (selectedComponent) {
            //ImGui::Text("%s", selectedComponent->GetName());
            selectedComponent->DrawImGui();
        }
        else {
            ImGui::Text("No component selected.");
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void EngineUI::ClearFramePreviews(Animation& anim) {
    for (auto& branch : anim.data.branches) {
        for (auto& frame : branch.frames) {
            if (frame.previewTexture != 0) {
                glDeleteTextures(1, &frame.previewTexture);
                frame.previewTexture = 0;
            }
        }
    }
}

void EngineUI::ApplyOverrides(AnimationFrame& frame, Agent& agent) {
    auto overrides = frame.componentOverrides;
    for (const auto& mod : overrides) {
        if (Component* comp = agent.GetComponentByID(mod.componentID)) {
            comp->SetEnabled(mod.enabled);
            comp->transform.position = mod.positionOffset;
            comp->transform.scale = mod.scale;
            comp->transform.rotation = mod.rotation;
            if (!mod.texturePath.empty() && std::string(comp->GetType()) == "SpriteComponent") {
                static_cast<SpriteComponent*>(comp)->SetTexturePath(mod.texturePath);
            }
        }
    }
    frame.dirty = true;
}

void EngineUI::ResetOverrides() {
    if (!pureAgent || !baseAgent) return;

    if (selectedBranch >= 0 && selectedBranch < currentAnimation.data.branches.size()) {
        auto& branch = currentAnimation.data.branches[selectedBranch];
        if (selectedFrame >= 0 && selectedFrame < branch.frames.size()) {
            auto& frame = branch.frames[selectedFrame];

            for (auto& mod : frame.componentOverrides) {
                Component* pure = pureAgent->GetComponentByID(mod.componentID);
                if (pure) {
                    mod.enabled = pure->IsEnabled();
                    mod.positionOffset = pure->transform.position;
                    mod.scale = pure->transform.scale;
                    mod.rotation = pure->transform.rotation;
                }
            }

            frame.dirty = true;
        }
    }
}

void EngineUI::SetupAnimation(const std::string pendingAgentFile) {
    baseAgent = std::make_unique<PlayerAgent>("Loaded");
    baseAgent->LoadFromFile(pendingAgentFile);

    pureAgent = std::make_unique<PlayerAgent>("Pure");
    pureAgent->LoadFromFile(pendingAgentFile);

    currentBaseAgentFile = pendingAgentFile;
    currentAnimation.data.defaultAgentPath = pendingAgentFile;

    // Reset animation-specific data
    selectedBranch = -1;
    selectedFrame = -1;
    ClearFramePreviews(currentAnimation);
    currentAnimation.data.defaultAgentPath = pendingAgentFile;
}

void EngineUI::RenderAnimationOverrides(AnimationFrame& frame, ComponentMod& mod) {
    auto& comp = *baseAgent->GetComponentByID(mod.componentID);
    frame.dirty = comp.DrawAnimatorImGui(mod);
}

void EngineUI::RenderAnimatorEditor(Scene& scene) {
    float width = ImGui::GetIO().DisplaySize.x;
    float height = ImGui::GetIO().DisplaySize.y;
    float topHeight = height * 0.5f;
    float leftWidth = width * 0.66f;
    float rightWidth = width - leftWidth;

    // -- Timeline (Top Left 2/3) --
    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSize(ImVec2(leftWidth, topHeight));
    ImGui::Begin("Timeline", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    //Agent injector
    ImGui::Text("Base Agent: %s", currentBaseAgentFile.empty() ? "None" : currentBaseAgentFile.c_str());
    if (ImGui::Button("Select Base Agent")) {
        ImGui::OpenPopup("AgentFileSelect");
    }

    if (ImGui::BeginPopup("AgentFileSelect")) {
        for (const auto& entry : std::filesystem::directory_iterator(AGENTS_FOLDER)) {
            if (entry.path().extension() == ".agent") {
                if (ImGui::Selectable(entry.path().filename().string().c_str())) {
                    pendingAgentFile = entry.path().string();
                    confirmAgentSwitch = true;
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        ImGui::EndPopup();
    }

    //Agent confirmation
    if (confirmAgentSwitch) {
        ImGui::OpenPopup("ConfirmAgentSwitch");
        confirmAgentSwitch = false;
    }

    if (ImGui::BeginPopupModal("ConfirmAgentSwitch", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Switching base agent will reset all animation data.\nContinue?");
        if (ImGui::Button("Yes")) {
            SetupAnimation(pendingAgentFile);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            pendingAgentFile.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();
    ImGui::Text("Global Flags");
    for (int i = 0; i < currentAnimation.data.flags.size(); ++i) {
        ImGui::PushID(i);
        char buffer[64];
        strncpy_s(buffer, currentAnimation.data.flags[i].c_str(), sizeof(buffer));
        if (ImGui::InputText("##Flag", buffer, sizeof(buffer))) {
            currentAnimation.data.flags[i] = buffer;
        }
        ImGui::SameLine();
        if (ImGui::Button("X")) {
            currentAnimation.data.flags.erase(currentAnimation.data.flags.begin() + i);
            ImGui::PopID();
            break;
        }
        ImGui::PopID();
    }
    if (ImGui::Button("Add Flag")) {
        currentAnimation.data.flags.push_back("NewFlag");
    }

    bool defaultSelected = (selectedBranch == -1);
    ImGui::Separator();
    ImGui::Text("States");
    ImGui::Indent();
    if (ImGui::Selectable("Default State", defaultSelected)) {
        selectedBranch = -1;
        selectedFrame = -1;
    }
    for (size_t i = 0; i < currentAnimation.data.branches.size(); ++i) {
        ImGui::PushID((int)i);
        bool selected = selectedBranch == (int)i;

        if (ImGui::Selectable(currentAnimation.data.branches[i].name.c_str(), selected)) {
            selectedBranch = (int)i;
            selectedFrame = 0;
        }
        
        static bool openRenamePopup = false; // Declare at top of your loop

        if (ImGui::BeginPopupContextItem("BranchContext")) {
            if (ImGui::MenuItem("Rename")) {
                openRenamePopup = true;  // Set flag instead of opening now
            }
            if (ImGui::MenuItem("Duplicate")) {
                // TODO: Deep copy branch logic
            }
            if (ImGui::MenuItem("Delete")) {
                currentAnimation.data.branches.erase(currentAnimation.data.branches.begin() + i);
                if (selectedBranch == (int)i) selectedBranch = -1;
            }
            ImGui::EndPopup();
        }

        // Must be outside of BeginPopupContextItem block
        if (openRenamePopup) {
            ImGui::OpenPopup("RenameBranch");
            openRenamePopup = false;
        }

        if (ImGui::BeginPopup("RenameBranch")) {
            static char renameBuffer[64] = "";
            strncpy_s(renameBuffer, currentAnimation.data.branches[i].name.c_str(), sizeof(renameBuffer));
            if (ImGui::InputText("New Name", renameBuffer, sizeof(renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                currentAnimation.data.branches[i].name = renameBuffer;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        
        ImGui::PopID();
    }
    ImGui::Unindent();

    if (ImGui::Button("Add Branch")) {
        AnimationBranch newBranch = { "NewBranch", {} };
        currentAnimation.data.branches.push_back(newBranch);
        selectedBranch = (int)currentAnimation.data.branches.size() - 1;
        selectedFrame = 0;
    }

    if(selectedBranch >= 0 && selectedBranch < currentAnimation.data.branches.size()) {
        auto& branch = currentAnimation.data.branches[selectedBranch];
        ImGui::Separator();

        ImGui::Text("Entry Conditions:");
        for(size_t i = 0; i < branch.conditions.size(); ++i) {
            ImGui::PushID((int)i);
            if (ImGui::BeginCombo("Flag", branch.conditions[i].flag.c_str())) {
                for (auto& flag : currentAnimation.data.flags) {
                    if (ImGui::Selectable(flag.c_str(), flag == branch.conditions[i].flag)) {
                        branch.conditions[i].flag = flag;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            ImGui::Checkbox("Expected", &branch.conditions[i].expectedValue);
            ImGui::SameLine();
            if(ImGui::Button("X")) {
                branch.conditions.erase(branch.conditions.begin() + i);
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }
        if (ImGui::Button("Add Entry Condition")) {
            branch.conditions.push_back({ "NewFlag", true });
        }

        ImGui::Separator();
        ImGui::Text("Frames in '%s'", branch.name.c_str());
        ImGui::BeginChild("FrameList", ImVec2(0, 100), true, ImGuiWindowFlags_HorizontalScrollbar);
        for(int i = 0; i < branch.frames.size(); ++i) {
            ImGui::PushID(i);
            bool isSelected = (i == selectedFrame);
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.8f, 1.0f));
            }

            if (ImGui::ImageButton(
                ("frame" + std::to_string(i)).c_str(),
                (ImTextureID)(intptr_t)GetFramePreviewTexture(branch.frames[i], *baseAgent),
                ImVec2(64, 64),
                ImVec2(0, 1),  // UV top-left
                ImVec2(1, 0)   // UV bottom-right (flipped Y)
            )) {
                selectedFrame = i;
            }

            if (isSelected) {
                ImGui::PopStyleColor();
            }

            if (ImGui::BeginPopupContextItem("FrameContext")) {
                if (ImGui::MenuItem("Duplicate")) {
                    // TODO: Deep copy of AnimationFrame
                }
                if (ImGui::MenuItem("Delete")) {
                    branch.frames.erase(branch.frames.begin() + i);
                    if (selectedFrame == i) selectedFrame = -1;
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Reset Frame Overrides")) {
                    ResetOverrides();
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::PopID();
        }
        ImGui::EndChild();
        if (ImGui::Button("Add Frame")) {
            AnimationFrame newFrame;
            if (baseAgent) {
                for (auto& comp : baseAgent->GetComponents()) {
                    newFrame.componentOverrides.push_back(ComponentMod{
                        comp->GetID(), comp->IsEnabled(), comp->transform.position, 
                        comp->transform.scale, comp->transform.rotation
                        });
                    newFrame.dirty = true;
                }
            }
            branch.frames.push_back(std::move(newFrame));
            selectedFrame = (int)branch.frames.size() - 1;
        }
    }

    ImGui::End();

    // -- Agent View (Top Right 1/3) --
    ImGui::SetNextWindowPos(ImVec2(leftWidth, 20));
    ImGui::SetNextWindowSize(ImVec2(rightWidth, topHeight));
    ImGui::Begin("Agent View", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    if (baseAgent) {
        ImVec2 regionSize = ImGui::GetContentRegionAvail();
        ImVec2 screenPos = ImGui::GetCursorScreenPos();

        // Apply component overrides
        if (selectedBranch >= 0 && selectedBranch < currentAnimation.data.branches.size()) {
            auto& branch = currentAnimation.data.branches[selectedBranch];
            if (selectedFrame >= 0 && selectedFrame < branch.frames.size()) {
                ApplyOverrides(branch.frames[selectedFrame], *baseAgent);
            }
        }

        // Render texture to match region size, aspect-preserved
        GLuint previewTex = 0;
        if (selectedBranch >= 0 && selectedBranch < currentAnimation.data.branches.size()) {
            auto& branch = currentAnimation.data.branches[selectedBranch];
            if (selectedFrame >= 0 && selectedFrame < branch.frames.size()) {
                AnimationFrame& frame = branch.frames[selectedFrame];

                // Apply overrides to baseAgent live
                ApplyOverrides(frame, *baseAgent);

                previewTex = (GLuint)(intptr_t)GetFramePreviewTexture(frame, *baseAgent);
            }
        }
        else {
            if (!baseAgent && !currentAnimation.data.defaultAgentPath.empty()) {
                baseAgent = std::make_unique<PlayerAgent>("Loaded");
                baseAgent->LoadFromFile(currentAnimation.data.defaultAgentPath);
            }

            previewTex = baseAgent->RenderAgentToTexture(256, 256);
        }
        float texAspect = (float)regionSize.x / (float)regionSize.y;

        // Maintain square preview centered in available space
        float previewSize = std::min(regionSize.x, regionSize.y);
        ImVec2 previewDims = ImVec2(previewSize, previewSize);
        ImVec2 previewPos = ImVec2(
            screenPos.x + (regionSize.x - previewSize) * 0.5f,
            screenPos.y + (regionSize.y - previewSize) * 0.5f
        );

        ImGui::SetCursorScreenPos(previewPos);
        ImGui::Image((ImTextureID)(intptr_t)previewTex, previewDims, ImVec2(0, 1), ImVec2(1, 0));

        // -- Mouse Interaction: Dragging, synced with ComponentMod --
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
            ImVec2 mouse = ImGui::GetMousePos();
            ImVec2 imageMin = previewPos;
            ImVec2 imageSize = previewDims;

            // Get mouse position relative to center of image
            float relativeX = (mouse.x - imageMin.x) - previewDims.x * 0.5f;
            float relativeY = (mouse.y - imageMin.y) - previewDims.y * 0.5f;

            // Convert to world space assuming ortho space from -256 to +256
            constexpr float pixelsPerUnit = 64.0f;
            float orthoScale = 1.0f / pixelsPerUnit; // world units per screen pixel (since it’s square)
            glm::vec2 worldMouse = {
                relativeX * orthoScale,
                -relativeY * orthoScale
            };

            bool mouseDown = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK;
            for (auto& comp : baseAgent->GetComponents()) {
                comp->HandleMouse(worldMouse, mouseDown);
            }

            // Sync updated transform back to override
            if (selectedBranch >= 0 && selectedBranch < currentAnimation.data.branches.size()) {
                auto& branch = currentAnimation.data.branches[selectedBranch];
                if (selectedFrame >= 0 && selectedFrame < branch.frames.size()) {
                    AnimationFrame& frame = branch.frames[selectedFrame];
                    for (auto& mod : frame.componentOverrides) {
                        Component* comp = baseAgent->GetComponentByID(mod.componentID);
                        if (comp) {
                            mod.positionOffset = comp->transform.position;
                            mod.scale = comp->transform.scale;
                            mod.rotation = comp->transform.rotation;
                            frame.dirty = true;
                        }
                    }
                }
            }
        }
    }
    else {
        ImGui::Text("No base agent set.");
    }
    ImGui::End();

    // -- Frame Editor (Bottom Half) --
    ImGui::SetNextWindowPos(ImVec2(0, 20 + topHeight));
    ImGui::SetNextWindowSize(ImVec2(width, height - topHeight - 20));
    ImGui::Begin("Frame Editor", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    if (selectedBranch >= 0 && selectedBranch < currentAnimation.data.branches.size()) {
        auto& branch = currentAnimation.data.branches[selectedBranch];
        if (selectedFrame >= 0 && selectedFrame < branch.frames.size()) {
            AnimationFrame& frame = branch.frames[selectedFrame];

            ImGui::PushItemWidth(100);
            ImGui::DragFloat("Frame Duration", &frame.duration, 0.1f, 0.01f, 10.0f);
            ImGui::PopItemWidth();

            ImGui::Separator();

            int indexToDelete = -1;

            for (int i = 0; i < frame.componentOverrides.size(); ++i) {
                auto& mod = frame.componentOverrides[i];
                ImGui::PushID((int)mod.componentID);

                if (Component* comp = baseAgent->GetComponentByID(mod.componentID)) {
                    const std::string& compName = comp->GetName();
                    ImGui::Text("%s", compName.c_str());
                }
                else {
                    ImGui::Text("Missing Component (ID %llu)", mod.componentID);
                }
                ImGui::SameLine();
                if (ImGui::Button("X")) {
                    ImGui::OpenPopup("ConfirmDeleteComponent");
                }

                if (ImGui::BeginPopupModal("ConfirmDeleteComponent", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Remove component override: %s?", baseAgent->GetComponentByID(mod.componentID)->GetName());
                    if (ImGui::Button("Yes")) {
                        indexToDelete = i;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                RenderAnimationOverrides(frame, mod);

                ImGui::Separator();

                ImGui::PopID();
            }

            // Outside the loop — erase if needed
            if (indexToDelete >= 0 && indexToDelete < frame.componentOverrides.size()) {
                frame.componentOverrides.erase(frame.componentOverrides.begin() + indexToDelete);
                frame.dirty = true;
            }

            ImGui::Text("Flag Changes:");
            for (size_t i = 0; i < frame.flagChanges.size(); ++i) {
                auto& flag = frame.flagChanges[i];
                ImGui::PushID((int)i);

                if (ImGui::BeginCombo("Flag", frame.flagChanges[i].flag.c_str())) {
                    for (auto& flag : currentAnimation.data.flags) {
                        if (ImGui::Selectable(flag.c_str(), flag == frame.flagChanges[i].flag)) {
                            frame.flagChanges[i].flag = flag;
                        }
                    }
                    ImGui::EndCombo();
                }

                ImGui::SameLine();
                ImGui::Checkbox("Set", &flag.set);

                ImGui::DragFloat("Duration", &flag.duration, 0.1f, 0.0f, 5.0f);

                if (ImGui::Button("Remove Flag")) {
                    frame.flagChanges.erase(frame.flagChanges.begin() + i);
                    ImGui::PopID();
                    break;  // break to avoid using invalid iterator
                }

                ImGui::Separator();
                ImGui::PopID();
            }

            if (ImGui::Button("Add Flag Change")) {
                frame.flagChanges.push_back({ "NewFlag", true, 1.0f });
            }
        }
        else {
            ImGui::Text("No valid frame selected.");
        }
    }
    else {
        ImGui::Text("No branch selected.");
    }

    ImGui::End();
}

void EngineUI::RenderGameView(Scene& scene) {
    //TODO
}