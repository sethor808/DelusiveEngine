#include <filesystem>
#include "EngineUI.h"
#include "Renderer.h"
#include "ElysiumComponents.h"
#include "ElysiumUtils.h"
#include <glm/gtc/type_ptr.hpp>

EngineUI::EngineUI() {
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
        path = scenePath;
        break;
    case EditorMode::AgentEditor:
        path = agentPath;
        break;
    case EditorMode::AnimatorEditor:
        path = animationPath;
        break;
    }

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().stem().string();
            //filter by extension code
            if (entry.path().extension() == ".scene" || entry.path().extension() == ".agent") {
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
    for (const auto& mod : frame.componentOverrides) {
        Component* comp = tempAgent->GetComponentByName(mod.componentName);
        if (comp) {
            comp->SetEnabled(mod.enabled);
            comp->SetLocalTransform(mod.positionOffset, mod.scale, mod.rotation);
        }
    }

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
                currentMode = EditorMode::SceneEditor;
                selectedAsset = "None";
                loadedAssets = LoadSceneList();
            }
            if (ImGui::Selectable("Agent Editor", currentMode == EditorMode::AgentEditor)) {
                currentMode = EditorMode::AgentEditor;
                selectedAsset = "None";
                scene = Scene("Agent Editor");
                loadedAssets = LoadSceneList();
            }
            if (ImGui::Selectable("Animator", currentMode == EditorMode::AnimatorEditor)) {
                currentMode = EditorMode::AnimatorEditor;
                selectedAsset = "None";
                scene = Scene("Animator");
                loadedAssets = LoadSceneList(); // This should point to .anim files
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
            if (ImGui::Selectable("Add New...")) {
                newAssetPopup = true;
            }

            for (const auto& asset : loadedAssets) {
                bool isSelected = (selectedAsset == asset);
                ImGui::PushID(asset.c_str());
                if (ImGui::Selectable(asset.c_str(), isSelected)) {
                    selectedAsset = asset;
                    std::string fullPath;
                    switch (currentMode) {
                    case EditorMode::SceneEditor: {
                        fullPath = scenePath + asset + sceneExtension;
                        scene = Scene(asset);
                        scene.LoadFromFile(fullPath);
                        break;
                    }
                    case EditorMode::AgentEditor: {
                        fullPath = agentPath + asset + agentExtension;
                        scene = Scene("TempAgentEdit");

                        std::ifstream in(fullPath);
                        if (in.is_open()) {
                            auto agent = std::make_unique<PlayerAgent>("Temp");
                            agent->LoadFromFile(in);
                            scene.AddAgent(std::move(agent));
                        }
                        break;
                    }
                    case EditorMode::AnimatorEditor: {
                        fullPath = animationPath + asset + animationExtension;

                        // Cleanup old previews
                        for (auto& branch : currentAnimation.data.branches) {
                            for (auto& frame : branch.frames) {
                                if (frame.previewTexture != 0) {
                                    glDeleteTextures(1, &frame.previewTexture);
                                    frame.previewTexture = 0;
                                }
                            }
                        }

                        currentAnimation = Animation(asset);
                        currentAnimation.LoadFromFile(fullPath);
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

            std::string savePath;
            switch (currentMode) {
            case EditorMode::SceneEditor: {
                savePath = scenePath + selectedAsset + sceneExtension;
                scene.SaveToFile(savePath);
                break;
            }
            case EditorMode::AgentEditor: {
                if (!scene.GetAgents().empty()) {
                    std::string savePath = agentPath + selectedAsset + agentExtension;
                    std::ofstream out(savePath);
                    if (out.is_open()) {
                        scene.GetAgents().front()->SaveToFile(out);  // Save the first (and only) agent
                        out.close();
                    }
                }
                break;
            }
            case EditorMode::AnimatorEditor: {
                savePath = animationPath + selectedAsset + animationExtension;
                currentAnimation.SaveToFile(savePath);
                break;
            }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Delete")) {
            showDeleteConfirm = true;
            assetToDelete = selectedAsset;
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 150.0f);
        if (ImGui::Button("Play/Stop")) {
            // Play toggle
        }

        ImGui::EndMainMenuBar();

        if (showDeleteConfirm) {
            ImGui::OpenPopup("Confirm Delete Scene");
        }

        if (ImGui::BeginPopupModal("Confirm Delete Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure you want to delete: %s", assetToDelete.c_str());

            if (ImGui::Button("Yes, Delete")) {
                std::string fullPath = (currentMode == EditorMode::SceneEditor)
                    ? scenePath + assetToDelete + sceneExtension
                    : agentPath + assetToDelete + agentExtension;

                std::remove(fullPath.c_str());
                loadedAssets = LoadSceneList();
                if (selectedAsset == assetToDelete) selectedAsset = "";
                assetToDelete.clear();
                showDeleteConfirm = false;
                ImGui::CloseCurrentPopup();
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
            std::string path = (currentMode == EditorMode::SceneEditor) ? scenePath : agentPath;
            std::string extension = (currentMode == EditorMode::SceneEditor) ? sceneExtension : agentExtension;

            ImGui::InputText("Name", assetNameBuffer, sizeof(assetNameBuffer));
            if (ImGui::Button("Create")) {
                if (strlen(assetNameBuffer) > 0) {
                    loadedAssets.push_back(assetNameBuffer);
                    selectedAsset = assetNameBuffer;

                    scene = Scene((currentMode == EditorMode::SceneEditor) ? assetNameBuffer : "Agent Editing View");

                    std::ofstream out(path + selectedAsset + extension);
                    out << "name " << selectedAsset << "\n";
                    out.close();

                    assetNameBuffer[0] = '\0';
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
    float topBarHeight = ImGui::GetFrameHeight();
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, topBarHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.3f, ImGui::GetIO().DisplaySize.y - topBarHeight), ImGuiCond_Always);
    ImGui::Begin("Scene Editor Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    if (ImGui::BeginChild("Hierarchy", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), true)) {
        ImGui::Text("Scene Hierarchy");
        if (ImGui::Button("Add Agent")) {
            scene.AddAgent(std::make_unique<PlayerAgent>("NewAgent"));
        }
        auto& agents = scene.GetAgents();
        for (size_t i = 0; i < agents.size(); ++i) {
            std::string name = agents[i]->GetName().empty() ? "Agent " + std::to_string(i) : agents[i]->GetName();
            ImGui::PushID((int)i);
            if (ImGui::Selectable(name.c_str(), selectedAgentIndex == (int)i)) {
                selectedAgentIndex = (int)i;
            }
            if (ImGui::BeginPopupContextItem(("AgentContextMenu" + std::to_string(i)).c_str())) {
                if (ImGui::MenuItem("Delete")) {
                    agents.erase(agents.begin() + i);
                    if (selectedAgentIndex == (int)i) selectedAgentIndex = -1;
                    else if (selectedAgentIndex > (int)i) selectedAgentIndex--;
                    ImGui::EndPopup();
                    ImGui::PopID();
                    break;
                }
                if (ImGui::BeginMenu("Load Prefab")) {
                    for (const auto& entry : std::filesystem::directory_iterator("assets/agents")) {
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
            ImGui::PopID();
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    if (ImGui::BeginChild("Inspector", ImVec2(0, 0), true)) {
        ImGui::Text("Inspector");
        auto& agents = scene.GetAgents();
        if (selectedAgentIndex >= 0 && selectedAgentIndex < (int)agents.size()) {
            Agent& selected = *agents[selectedAgentIndex];

            char nameBuffer[64];
            strncpy_s(nameBuffer, selected.GetName().c_str(), sizeof(nameBuffer));
            if (ImGui::InputText("Agent Name", nameBuffer, sizeof(nameBuffer))) {
                if (strlen(nameBuffer) > 0) {
                    selected.SetName(std::string(nameBuffer));
                }
                else {
                    // Restore or ignore empty name
                    strncpy_s(nameBuffer, selected.GetName().c_str(), sizeof(nameBuffer));
                }
            }

            glm::vec2 pos = selected.GetTransform().position;
            if (ImGui::DragFloat2("Position", glm::value_ptr(pos), 1.0f)) {
                selected.SetPosition(pos);
            }

            float rotation = selected.GetTransform().rotation;
            if (ImGui::DragFloat("Rotation", &rotation, 0.01f)) {
                selected.SetRotation(rotation);
            }

            glm::vec2 scale = selected.GetTransform().scale;
            if (ImGui::DragFloat2("Scale", glm::value_ptr(scale), 0.01f)) {
                selected.SetScale(scale);
            }

            ImGui::Separator();
            ImGui::Text("Components");

            auto& components = selected.GetComponents();
            for (auto& comp : components) {
                ImGui::PushID(comp.get());
                ImGui::Separator();
                ImGui::BeginGroup();
                ImGui::Text("%s", comp->GetName());
                if (ImGui::Button("Remove Component")) {
                    selected.RemoveComponentByPointer(comp.get());
                    ImGui::EndGroup();
                    ImGui::PopID();
                    break;
                }
                comp->DrawImGui();
                ImGui::EndGroup();
                ImGui::PopID();
            }

            if (ImGui::Button("Add Component")) {
                ImGui::OpenPopup("AddComponentPopup");
            }

            if (ImGui::BeginPopup("AddComponentPopup")) {
                if (ImGui::MenuItem("Sprite")) selected.AddComponent<SpriteComponent>("assets/sprites/star.jpg");
                if (ImGui::BeginMenu("Collider")) {
                    if (ImGui::MenuItem("Solid")) selected.AddComponent<SolidCollider>();
                    if (ImGui::MenuItem("Hitbox")) selected.AddComponent<HitboxCollider>();
                    if (ImGui::MenuItem("Hurtbox")) selected.AddComponent<HurtboxCollider>();
                    if (ImGui::MenuItem("Trigger")) selected.AddComponent<TriggerCollider>();
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Stats")) selected.AddComponent<StatsComponent>();
                ImGui::EndPopup();
            }
        }
        else {
            ImGui::Text("No Agent Selected");
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

void EngineUI::RenderAgentEditor(Scene& scene) {
    //Mouse stuff
    static bool isDraggingCollider = false;
    if (!ImGui::GetIO().WantCaptureMouse) {
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        glm::vec2 worldMouse = ScreenToWorld2D(mouseX, mouseY);
        bool mouseDown = SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK;

        Agent& agent = *scene.GetAgents().front();
        for (auto& comp : agent.GetComponents()) {
            /*
            if (isDraggingCollider && comp.get() != currentlyDraggingComponent)
                continue;
            */
            if (isDraggingCollider)
                continue;

            comp->HandleMouse(worldMouse, mouseDown);
        }

        // Optionally, set currently dragging
        /*
        if (auto col = dynamic_cast<ColliderComponent*>(comp.get())) {
            if (col->IsDragging()) {
                isDraggingCollider = true;
                currentlyDraggingComponent = col;
            }
            else if (currentlyDraggingComponent == col && !mouseDown) {
                isDraggingCollider = false;
                currentlyDraggingComponent = nullptr;
            }
        }
        */
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

    static Component* selectedComponent = nullptr;
    static bool agentSelected = true;

    if (ImGui::BeginChild("Component List", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), true)) {
        if (ImGui::Selectable(agent.GetName().c_str(), agentSelected)) {
            selectedComponent = nullptr;
            agentSelected = true;
        } 

        if (ImGui::BeginPopupContextItem("AgentContext")) { 
            if (ImGui::BeginMenu("Add")) { 
                if (ImGui::MenuItem("Sprite")) {
                    selectedComponent = agent.AddComponent<SpriteComponent>("assets/sprites/star.jpg");
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
            if (ImGui::Selectable(comp->GetName(), isSelected)) {
                selectedComponent = comp.get();
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
        for (const auto& entry : std::filesystem::directory_iterator("assets/agents")) {
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
            baseAgent = std::make_unique<PlayerAgent>("Loaded");
            baseAgent->LoadFromFile(pendingAgentFile);
            currentBaseAgentFile = pendingAgentFile;
            currentAnimation.data.branches.clear();
            selectedBranch = -1;
            selectedFrame = -1;
            for (auto& branch : currentAnimation.data.branches) {
                for (auto& frame : branch.frames) {
                    if (frame.previewTexture != 0) {
                        glDeleteTextures(1, &frame.previewTexture);
                        frame.previewTexture = 0;
                    }
                }
            }
            currentAnimation.data.defaultState = AnimationFrame();
            if (baseAgent) {
                for (auto& comp : baseAgent->GetComponents()) {
                    currentAnimation.data.defaultState.componentOverrides.push_back(ComponentMod{
                        comp->GetName(), comp->IsEnabled(), comp->transform.position,
                        comp->transform.scale, comp->transform.rotation
                        });
                }
            }
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
                        comp->GetName(), comp->IsEnabled(), comp->transform.position, 
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
                for (const auto& mod : branch.frames[selectedFrame].componentOverrides) {
                    Component* comp = baseAgent->GetComponentByName(mod.componentName);
                    if (comp) {
                        comp->SetEnabled(mod.enabled);
                        comp->SetLocalTransform(mod.positionOffset, mod.scale, mod.rotation);
                    }
                }
            }
        }
        else {
            for (const auto& mod : currentAnimation.data.defaultState.componentOverrides) {
                Component* comp = baseAgent->GetComponentByName(mod.componentName);
                if (comp) {
                    comp->SetEnabled(mod.enabled);
                    comp->SetLocalTransform(mod.positionOffset, mod.scale, mod.rotation);
                }
            }
        }

        // Render texture to match region size, aspect-preserved
        GLuint previewTex = 0;
        if (selectedBranch >= 0 && selectedBranch < currentAnimation.data.branches.size()) {
            auto& branch = currentAnimation.data.branches[selectedBranch];
            if (selectedFrame >= 0 && selectedFrame < branch.frames.size()) {
                AnimationFrame& frame = branch.frames[selectedFrame];

                // Apply overrides to baseAgent live
                for (const auto& mod : frame.componentOverrides) {
                    Component* comp = baseAgent->GetComponentByName(mod.componentName);
                    if (comp) {
                        comp->SetEnabled(mod.enabled);
                        comp->SetLocalTransform(mod.positionOffset, mod.scale, mod.rotation);
                    }
                }

                previewTex = (GLuint)(intptr_t)GetFramePreviewTexture(frame, *baseAgent);
            }
        }
        else {
            for (const auto& mod : currentAnimation.data.defaultState.componentOverrides) {
                Component* comp = baseAgent->GetComponentByName(mod.componentName);
                if (comp) {
                    comp->SetEnabled(mod.enabled);
                    comp->SetLocalTransform(mod.positionOffset, mod.scale, mod.rotation);
                }
            }

            previewTex = (GLuint)(intptr_t)GetFramePreviewTexture(currentAnimation.data.defaultState, *baseAgent);
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
                        Component* comp = baseAgent->GetComponentByName(mod.componentName);
                        if (comp) {
                            mod.positionOffset = comp->transform.position;
                            mod.scale = comp->transform.scale;
                            mod.rotation = comp->transform.rotation;
                            frame.dirty = true;
                        }
                    }
                }
            }
            else {
                for (auto& mod : currentAnimation.data.defaultState.componentOverrides) {
                    Component* comp = baseAgent->GetComponentByName(mod.componentName);
                    if (comp) {
                        mod.positionOffset = comp->transform.position;
                        mod.scale = comp->transform.scale;
                        mod.rotation = comp->transform.rotation;
                        currentAnimation.data.defaultState.dirty = true;
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
                ImGui::PushID(mod.componentName.c_str());

                ImGui::Text("%s", mod.componentName.c_str());
                ImGui::SameLine();
                if (ImGui::Button("X")) {
                    ImGui::OpenPopup("ConfirmDeleteComponent");
                }

                if (ImGui::BeginPopupModal("ConfirmDeleteComponent", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Remove component override: %s?", mod.componentName.c_str());
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

                ImGui::Checkbox("Enabled", &mod.enabled);
                frame.dirty |= ImGui::IsItemEdited();

                frame.dirty |= ImGui::DragFloat2("Offset", glm::value_ptr(mod.positionOffset), 1.0f);
                frame.dirty |= ImGui::DragFloat2("Scale", glm::value_ptr(mod.scale), 0.01f);
                frame.dirty |= ImGui::DragFloat("Rotation", &mod.rotation, 0.01f);
                ImGui::Separator();

                ImGui::PopID();
            }

            // Outside the loop — erase if needed
            if (indexToDelete >= 0 && indexToDelete < frame.componentOverrides.size()) {
                frame.componentOverrides.erase(frame.componentOverrides.begin() + indexToDelete);
                frame.dirty = true;
            }

            ImGui::Separator();
            if (ImGui::Button("Add Component")) {
                ImGui::OpenPopup("AddOverridePopup");
            }

            if (ImGui::BeginPopup("AddOverridePopup")) {
                if (ImGui::BeginMenu("Sprite")) {
                    if (ImGui::MenuItem("Add SpriteComponent")) {
                        frame.componentOverrides.push_back({
                            "SpriteComponent", true, glm::vec2(0.0f), glm::vec2(1.0f), 0.0f
                            });
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Collider")) {
                    if (ImGui::MenuItem("Add SolidCollider")) {
                        frame.componentOverrides.push_back({
                            "SolidCollider", true, glm::vec2(0.0f), glm::vec2(1.0f), 0.0f
                            });
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Add HitboxCollider")) {
                        frame.componentOverrides.push_back({
                            "HitboxCollider", true, glm::vec2(0.0f), glm::vec2(1.0f), 0.0f
                            });
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Add HurtboxCollider")) {
                        frame.componentOverrides.push_back({
                            "HurtboxCollider", true, glm::vec2(0.0f), glm::vec2(1.0f), 0.0f
                            });
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem("Add TriggerCollider")) {
                        frame.componentOverrides.push_back({
                            "TriggerCollider", true, glm::vec2(0.0f), glm::vec2(1.0f), 0.0f
                            });
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Add StatsComponent")) {
                    frame.componentOverrides.push_back({
                        "StatsComponent", true, glm::vec2(0.0f), glm::vec2(1.0f), 0.0f
                        });
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::Text("Flag Changes:");
            for (size_t i = 0; i < frame.flagChanges.size(); ++i) {
                auto& flag = frame.flagChanges[i];
                ImGui::PushID(i);

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