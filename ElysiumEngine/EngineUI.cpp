#include <filesystem>
#include "EngineUI.h"

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
            scene.AddAgent(std::make_unique<PlayerAgent>("assets/sprites/star.png", "NewAgent"));
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