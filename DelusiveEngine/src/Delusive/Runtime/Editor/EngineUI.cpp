#include <filesystem>
#include <Delusive/Runtime/Editor/EngineUI.h>
#include <Delusive/Runtime/Agents/DelusiveAgents.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Components/DelusiveComponents.h>
#include <Delusive/Runtime/Utils/DelusiveUtils.h>
#include <Delusive/Runtime/Scene/DelusiveSystems.h>
#include <Delusive/Runtime/Components/TransformComponent.h>
#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/Editor/AnimatorEditor.h>
#include <Delusive/Runtime/Editor/SceneEditor.h>
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

EngineUI::EngineUI(GameManager& game)
    : gameManager(game), instance(game.GetInstance())
{
    //Property pickers reach the factories through this
    DelusiveEditorContext::SetInstance(&instance);

    animatorEditor = std::make_unique<AnimatorEditor>(instance);
    sceneEditor = std::make_unique<SceneEditor>(instance);

    currentMode = EditorMode::SceneEditor;
    loadedAssets = LoadSceneList();
}

EngineUI::~EngineUI() = default;

void EngineUI::LinkEditorCamera(CameraAgent* cam) {
    editorCamera = cam;
    sceneEditor->LinkEditorCamera(cam);
}

const char* ViewModeToString(EditorMode mode) {
    switch (mode) {
    case EditorMode::SceneEditor: return "Scene Editor";
    case EditorMode::AgentEditor: return "Agent Editor";
    case EditorMode::AnimatorEditor: return "Animator";
    case EditorMode::UIBuilder: return "UI Builder";
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

void EngineUI::Render(Scene& topBarScene) {
    RenderTopBar(topBarScene);
    //The top bar may have stopped play - from here on edit whichever scene is live now
    Scene& scene = gameManager.GetActiveScene();

    if (currentMode == EditorMode::SceneEditor) sceneEditor->RenderStatusBar(scene, gameManager.IsPlaying());
    DockSpace();
    Shortcuts(scene);

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
    case EditorMode::UIBuilder:
        RenderUIBuilder(scene);
        break;
    case EditorMode::GameView:
        RenderGameView(scene);
        break;
    }
}

void EngineUI::DockSpace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImGuiID dockID = ImGui::GetID(ViewModeToString(currentMode));
    //Only when the layout has never been arranged - after that imgui.ini keeps it
    if (!ImGui::DockBuilderGetNode(dockID)) BuildDefaultLayout(dockID);
    ImGui::DockSpaceOverViewport(dockID, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
}

void EngineUI::BuildDefaultLayout(ImGuiID dockID) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockBuilderAddNode(dockID, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodePos(dockID, viewport->WorkPos);
    ImGui::DockBuilderSetNodeSize(dockID, viewport->WorkSize);

    //Each split takes its fraction of what is left in the middle
    ImGuiID centre = dockID;
    auto Split = [&centre](ImGuiDir dir, float fraction) {
        return ImGui::DockBuilderSplitNode(centre, dir, fraction, nullptr, &centre);
    };

    switch (currentMode) {
    case EditorMode::SceneEditor: {
        const ImGuiID left = Split(ImGuiDir_Left, 0.18f);
        const ImGuiID right = Split(ImGuiDir_Right, 0.34f);
        ImGui::DockBuilderDockWindow(SceneEditor::HierarchyWindow, left);
        ImGui::DockBuilderDockWindow(SceneEditor::InspectorWindow, right);
        break;
    }
    case EditorMode::AnimatorEditor: {
        const ImGuiID bottom = Split(ImGuiDir_Down, 0.28f);
        const ImGuiID left = Split(ImGuiDir_Left, 0.22f);
        const ImGuiID right = Split(ImGuiDir_Right, 0.28f);
        ImGui::DockBuilderDockWindow(AnimatorEditor::SetWindow, left);
        ImGui::DockBuilderDockWindow(AnimatorEditor::FrameWindow, right);
        ImGui::DockBuilderDockWindow(AnimatorEditor::TimelineWindow, bottom);
        ImGui::DockBuilderDockWindow(AnimatorEditor::CanvasWindow, centre);
        break;
    }
    case EditorMode::AgentEditor:
        ImGui::DockBuilderDockWindow(AgentPanelWindow, Split(ImGuiDir_Right, 0.3f));
        break;
    case EditorMode::UIBuilder:
        ImGui::DockBuilderDockWindow(UIBuilderPanelWindow, Split(ImGuiDir_Right, 0.3f));
        break;
    default:
        break;
    }
    ImGui::DockBuilderFinish(dockID);
}

bool EngineUI::CanUndo() const {
    switch (currentMode) {
    case EditorMode::SceneEditor: return !gameManager.IsPlaying() && sceneEditor->CanUndo();
    case EditorMode::AnimatorEditor: return animatorEditor->CanUndo();
    default: return false;
    }
}

bool EngineUI::CanRedo() const {
    switch (currentMode) {
    case EditorMode::SceneEditor: return !gameManager.IsPlaying() && sceneEditor->CanRedo();
    case EditorMode::AnimatorEditor: return animatorEditor->CanRedo();
    default: return false;
    }
}

void EngineUI::Undo(Scene& scene) {
    if (!CanUndo()) return;
    if (currentMode == EditorMode::SceneEditor) sceneEditor->Undo(scene);
    else if (currentMode == EditorMode::AnimatorEditor) animatorEditor->Undo();
}

void EngineUI::Redo(Scene& scene) {
    if (!CanRedo()) return;
    if (currentMode == EditorMode::SceneEditor) sceneEditor->Redo(scene);
    else if (currentMode == EditorMode::AnimatorEditor) animatorEditor->Redo();
}

void EngineUI::Shortcuts(Scene& scene) {
    //Text fields keep their own Ctrl+Z
    if (ImGui::GetIO().WantTextInput) return;
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y) || ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z)) Redo(scene);
    else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z)) Undo(scene);
}

void EngineUI::SwitchMode(Scene& scene, EditorMode mode) {
    currentMode = mode;
    selectedAsset = "None";

    selectedComponent = nullptr;
    animatorEditor->Close();

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
    case EditorMode::UIBuilder:
        scene.SetName("UI Builder");
        selectedUIElement = nullptr;
        break;
    default:
        scene.SetName("New Scene");
    }
    sceneEditor->SceneLoaded(scene);

    loadedAssets = LoadSceneList();
}

void EngineUI::OpenAsset(Scene& scene, const std::string& asset) {
    selectedAsset = asset;
    std::string fullPath = GetPath(asset);
    switch (currentMode) {
    case EditorMode::SceneEditor: {
        scene.LoadFromFile(fullPath);
        sceneEditor->SceneLoaded(scene);
        break;
    }
    case EditorMode::AgentEditor: {
        LoadAgentAsset(scene, fullPath);
        break;
    }
    case EditorMode::AnimatorEditor: {
        animatorEditor->Open(fullPath);
        break;
    }
    default: break;
    }
}

void EngineUI::StartIn(Scene& scene, EditorMode mode, const std::string& asset) {
    SwitchMode(scene, mode);
    if (!asset.empty()) OpenAsset(scene, asset);
}

void EngineUI::LoadAgentAsset(Scene& scene, const std::string& path) {
    //Every Agent/Component pointer into the old agent dies here
    scene.ClearAgents();
    if (selectedComponent) selectedComponent = nullptr;
    agentSelected = true;

    if (auto agent = Agent::LoadFromFile(path, instance, &scene)) {
        scene.AddAgent(std::move(agent));
    }
    //On failure RenderAgentEditor falls back to a blank PlayerAgent
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

void EngineUI::MoveEditorCameraTo(Agent* agent) {
    if (!agent || !editorCamera) return;

    editorCamera->SetPan(agent->GetTransform().position);
}

void EngineUI::RenderTopBar(Scene& scene) {
    if (ImGui::BeginMainMenuBar()) {
        ImGui::Text("View");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(175.0f);
        if (ImGui::BeginCombo("##ViewModeSelector", ViewModeToString(currentMode))) {
            StopPlaying(scene);
            Scene& editing = gameManager.GetActiveScene();
            if (ImGui::Selectable("Scene Editor", currentMode == EditorMode::SceneEditor)) {
                SwitchMode(editing, EditorMode::SceneEditor);
            }
            if (ImGui::Selectable("Agent Editor", currentMode == EditorMode::AgentEditor)) {
                SwitchMode(editing, EditorMode::AgentEditor);
            }
            if (ImGui::Selectable("Animator", currentMode == EditorMode::AnimatorEditor)) {
                SwitchMode(editing, EditorMode::AnimatorEditor);
            }
            if (ImGui::Selectable("UI Builder", currentMode == EditorMode::UIBuilder)) {
                SwitchMode(editing, EditorMode::UIBuilder);
            }
            if (ImGui::Selectable("Game View", currentMode == EditorMode::GameView)) {
                currentMode = EditorMode::GameView;
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine(0.0f, 10.0f);
        const char* assetLabel = (currentMode == EditorMode::SceneEditor) ? "Loaded Scene: " : "Loaded Asset: ";
        ImGui::Text("%s", assetLabel);

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SameLine();
        if (ImGui::BeginCombo("##AssetSelector", selectedAsset.c_str())) {
            StopPlaying(scene);
            Scene& editing = gameManager.GetActiveScene();

            if (ImGui::Selectable("Add New...")) {
                newAssetPopup = true;
            }

            for (const auto& asset : loadedAssets) {
                bool isSelected = (selectedAsset == asset);
                ImGui::PushID(asset.c_str());
                if (ImGui::Selectable(asset.c_str(), isSelected)) {
                    OpenAsset(editing, asset);
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

        //The play copy is never saved or undone - Stop returns to the scene being edited
        const bool playing = gameManager.IsPlaying();
        ImGui::SameLine();
        ImGui::BeginDisabled(playing);
        if (ImGui::Button("Save")) {
            std::string path;
            std::string name = selectedAsset.empty() ? "Unnamed" : selectedAsset;

            std::string savePath = GetPath(selectedAsset);
            switch (currentMode) {
            case EditorMode::SceneEditor: {
                bool reloaded = false;
                if (scene.SaveToFile(savePath, &reloaded)) {
                    //A copy saved under a new name carries fresh ids - its history starts here
                    if (reloaded) sceneEditor->SceneLoaded(scene);
                    else sceneEditor->SceneSaved(scene);
                }
                break;
            }
            case EditorMode::AgentEditor: {
                if (!scene.GetAgents().empty()) {
                    bool remapped = false;
                    //Saved as a copy - reload so the live agent carries the copy's fresh ids
                    if (scene.GetAgents().front()->SaveToFile(savePath, &remapped) && remapped) {
                        LoadAgentAsset(scene, savePath);
                    }
                }
                break;
            }
            case EditorMode::AnimatorEditor: {
                animatorEditor->Save(savePath);
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

        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x * 3.0f);
        ImGui::BeginDisabled(!CanUndo());
        if (ImGui::Button("Undo")) Undo(scene);
        ImGui::SetItemTooltip("Ctrl+Z");
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(!CanRedo());
        if (ImGui::Button("Redo")) Redo(scene);
        ImGui::SetItemTooltip("Ctrl+Y / Ctrl+Shift+Z");
        ImGui::EndDisabled();
        ImGui::EndDisabled();

        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x * 3.0f);
        if (currentMode == EditorMode::SceneEditor || currentMode == EditorMode::GameView) {
            if (ImGui::Button(playing ? "Stop" : "Play")) {
                if (playing) StopPlaying(scene);
                else {
                    sceneEditor->ClearSelection(scene);
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
                gameManager.GetInstance().delusiveLibrary.RemoveFile(fullPath);
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

                    if (currentMode == EditorMode::SceneEditor) {
                        scene.SaveToFile(GetPath(selectedAsset));
                        sceneEditor->SceneLoaded(scene);
                    }
                    else if (currentMode == EditorMode::AgentEditor) {
                        auto agent = std::make_unique<PlayerAgent>(instance);
                        agent->SetName(selectedAsset);
                        agent->SaveToFile(GetPath(selectedAsset));
                        scene.AddAgent(std::move(agent));
                    }
                    else if (currentMode == EditorMode::AnimatorEditor) {
                        animatorEditor->Create(GetPath(selectedAsset), selectedAsset);
                    }

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

void EngineUI::StopPlaying(Scene& scene) {
    if (!gameManager.IsPlaying()) return;
    //Selection lives by id, but the editor-mode flags it set belong to the play copy
    sceneEditor->ClearSelection(scene);
    gameManager.Stop();
}

void EngineUI::RenderSceneEditor(Scene& scene) {
    sceneEditor->Render(scene, gameManager.IsPlaying());
}

void EngineUI::RenderAgentEditor(Scene& scene) {
    //Mouse stuff
    if (!ImGui::GetIO().WantCaptureMouse) {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        glm::vec2 worldMouse = ScreenToWorld2D((int)mouseX, (int)mouseY, instance.renderer.GetProjection());
        bool mouseDown = SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK;

        //The agent is created further down on the first frame after switching modes
        if (!scene.GetAgents().empty()) {
            Agent& agent = *scene.GetAgents().front();

            //Collider handles win over dragging whatever sits underneath them
            isDraggingCollider = false;
            for (auto& comp : agent.GetComponents()) {
                if (auto* collider = dynamic_cast<ColliderComponent*>(comp.get())) {
                    collider->HandleMouse(worldMouse, mouseDown);
                    isDraggingCollider |= collider->IsDraggingHandle();
                }
            }
            for (auto& comp : agent.GetComponents()) {
                if (isDraggingCollider) break;
                if (!dynamic_cast<ColliderComponent*>(comp.get())) comp->HandleMouse(worldMouse, mouseDown);
            }
        }
    }

    float topBarHeight = ImGui::GetFrameHeight();
    /*
    * OLD BEFORE REDOCKABLES
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, topBarHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.3f, ImGui::GetIO().DisplaySize.y - topBarHeight), ImGuiCond_Always);
    ImGui::Begin("Agent Editor Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    */

    ImGui::Begin(AgentPanelWindow);

    auto& agents = scene.GetAgents();
    if (agents.empty()) {
        scene.AddAgent(std::make_unique<PlayerAgent>(instance));
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
                    selectedComponent = agent.AddComponent<SpriteComponent>();
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
            std::snprintf(nameBuffer, sizeof(nameBuffer), "%s", agent.GetName().c_str());
            ImGui::Text("Name");
            ImGui::SameLine();
            if (ImGui::InputText("##nameInput", nameBuffer, sizeof(nameBuffer))) {
                if (strlen(nameBuffer) > 0) {
                    agent.SetName(std::string(nameBuffer));
                }
                else {
                    // Restore or ignore empty name
                    std::snprintf(nameBuffer, sizeof(nameBuffer), "%s", agent.GetName().c_str());
                }
            }

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

void EngineUI::RenderAnimatorEditor(Scene&) {
    animatorEditor->Render();
}

void EngineUI::RenderGameView(Scene& scene) {
    //TODO
}