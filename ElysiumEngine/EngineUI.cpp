#include <filesystem>
#include "EngineUI.h"
#include "ElysiumComponents.h"

EngineUI::EngineUI() {
    currentMode = EditorMode::SceneEditor;
    loadedAssets = LoadSceneList();
}

EngineUI::~EngineUI() {

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
                    case EditorMode::SceneEditor:
                        fullPath = scenePath + asset + sceneExtension;
                        scene = Scene(asset);
                        scene.LoadFromFile(fullPath);
                        break;
                    case EditorMode::AgentEditor:
                        fullPath = agentPath + asset + agentExtension;
                        scene = Scene("TempAgentEdit");
                        //scene.LoadFromFile(fullPath);
                        break;
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
            // Save logic
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
                selected.SetName(std::string(nameBuffer));
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

            static bool showColliders = false;
            if (ImGui::Checkbox("Edit Colliders", &showColliders)) {
                // Link to collider toggle
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
                if (ImGui::MenuItem("Collider")) {
                    selectedComponent = agent.AddComponent<SolidCollider>();
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
                agent.SetName(std::string(nameBuffer));
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
    //TODO
}

void EngineUI::RenderGameView(Scene& scene) {
    //TODO
}

/*
void EngineUI::Render(Scene& scene) {
    static bool newAssetPopup = false;
    static std::string assetToDelete = "";
    static bool showDeleteConfirm = false;

    if (ImGui::BeginMainMenuBar()) {
        //View mode buttons
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
                    case EditorMode::SceneEditor:
                        fullPath = scenePath + asset + sceneExtension;
                        scene = Scene(asset);  // reset the scene
                        scene.LoadFromFile(fullPath);
                        break;
                    case EditorMode::AgentEditor:
                        fullPath = agentPath + asset + agentExtension;
                        scene = Scene("TempAgentEdit");  // reset
                        scene.LoadFromFile(fullPath);    // treat like a single-agent scene
                        break;
                    default:
                        break;
                    }
                }

                //Right click menu
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
    
        // -- Save Button --
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            // TODO: Save current asset or scene
        }

        ImGui::SameLine();
        if (ImGui::Button("Delete")) {
            showDeleteConfirm = true;
            assetToDelete = selectedAsset;
        }


        ImGui::SameLine(ImGui::GetWindowWidth() - 150.0f);
        if (ImGui::Button("Play/Stop")) {
            //TODO: Toggle play mode
        }
        ImGui::EndMainMenuBar();
    }

    if (ImGui::BeginPopupModal("Confirm Delete Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete scene:\n\n%s\n\nThis action cannot be undone.", assetToDelete.c_str());

        if (ImGui::Button("Yes, Delete")) {
            std::string fullPath = "empty";

            if (currentMode == EditorMode::SceneEditor) {
                fullPath = scenePath + assetToDelete + ".scene";
            }
            else if(currentMode == EditorMode::AgentEditor){
                fullPath = agentPath + assetToDelete + ".agent";
            }
            
            if (fullPath == "empty") return;
            std::remove(fullPath.c_str());

            loadedAssets = LoadSceneList();
            if (selectedAsset == assetToDelete) {
                selectedAsset = "";
            }

            assetToDelete = "";
            showDeleteConfirm = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            assetToDelete = "";
            showDeleteConfirm = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (showDeleteConfirm) {
        ImGui::OpenPopup("Confirm Delete Scene");
    }

    if (ImGui::BeginPopup("NewAssetPopup")) {
        static char assetNameBuffer[64] = "";
        std::string path;
        std::string assetType;
        std::string fileType;

        switch (currentMode) {
        case EditorMode::SceneEditor:
            path = scenePath;
            assetType = "Scene";
            fileType = sceneExtension;
            break;
        case EditorMode::AgentEditor:
            path = agentPath;
            assetType = "Agent";
            fileType = agentExtension;
            break;
        default:
            return;
            break;
        }

        ImGui::InputText((assetType + " Name").c_str(), assetNameBuffer, sizeof(assetNameBuffer));
        if (ImGui::Button("Create")) {
            if (strlen(assetNameBuffer) > 0) {
                loadedAssets.push_back(assetNameBuffer);
                selectedAsset = assetNameBuffer;
                
                if (assetType == "Scene") {
                    scene = Scene(assetNameBuffer);
                } else if (assetType == "Agent") {
                    scene = Scene("Agent Editing View");
                }

                std::ofstream out(path + selectedAsset + fileType);
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
        loadedAssets = LoadSceneList();
        ImGui::EndPopup();
    }

    if (newAssetPopup) {
        ImGui::OpenPopup("NewAssetPopup");
        newAssetPopup = false;
    }

    //Right side docks
    float topBarHeight = ImGui::GetFrameHeight(); // returns ~20-25 pixels

    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, topBarHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.3f, ImGui::GetIO().DisplaySize.y - topBarHeight), ImGuiCond_Always);
    ImGui::Begin("Scene Editor Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    if (ImGui::BeginChild("Hierarchy", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 0), true)) {
        ImGui::Text("Scene Hierarchy");
        if (ImGui::Button("Add Agent")) {
            scene.AddAgent(std::make_unique<PlayerAgent>("assets/sprites/star.jpg", "NewAgent"));
        }
        auto& agents = scene.GetAgents();
        for (size_t i = 0; i < agents.size(); ++i) {
            std::string name = agents[i]->GetName().empty() ? "Agent " + std::to_string(i) : agents[i]->GetName();

            ImGui::PushID((int)i);
            if (ImGui::Selectable(name.c_str(), selectedAgentIndex == (int)i)) {
                selectedAgentIndex = (int)i;
            }

            //Right-click menu
            if (ImGui::BeginPopupContextItem(("AgentContextMenu" + std::to_string(i)).c_str())) {
                if (ImGui::MenuItem("Delete")) {
                    agents.erase(agents.begin() + i);
                    if (selectedAgentIndex == (int)i) {
                        selectedAgentIndex = -1;
                    }
                    else if(selectedAgentIndex > (int)i){
                        selectedAgentIndex--;
                    }
                    ImGui::EndPopup();
                    ImGui::PopID();
                    break;
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
                selected.SetName(std::string(nameBuffer));
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

            static bool showColliders = false;
            if (ImGui::Checkbox("Edit Colliders", &showColliders)) {
                //TODO: Link this into the collider rendering
            }
        } else {
            ImGui::Text("No Agent Selected");
        }
    }
    ImGui::EndChild();

    ImGui::End();
}
*/