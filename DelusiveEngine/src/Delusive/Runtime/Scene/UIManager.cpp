#include <Delusive/Runtime/Scene/UIManager.h>
#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <algorithm>
#include <iostream>
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>

UIManager::UIManager(DelusiveInstance& instance)
	: SceneSystem(instance), uiRegistry(instance)
{
	name = "NewUIManager";
	activeCanvas = nullptr;
    uiRegistry.LinkManager(this);
	RegisterProperties();
}

UIManager::~UIManager() {
    if (activeCanvas) {
		activeCanvas->DelinkManager();
    }
}

void UIManager::Init() {
    //Runs after Deserialize, so the canvas ids are already loaded
    uiRegistry.LoadAll();
    ResolveCanvases();
}

void UIManager::ResolveCanvases() {
    canvases.clear();
    activeCanvas = nullptr;

    for (const std::string& idText : canvasIDs) {
        UUID canvasID;
        canvasID.FromString(idText);

        if (UICanvas* canvas = uiRegistry.Get(canvasID)) {
            canvases.push_back(canvas);
        }
        else {
            std::cerr << "[UIManager] Missing canvas " << idText << std::endl;
        }
    }

    if (UICanvas* canvas = uiRegistry.Get(activeCanvasID)) {
        ActivateCanvas(canvas);
    }
}

void UIManager::SyncCanvasIDs() {
    canvasIDs.clear();
    for (UICanvas* canvas : canvases) {
        if (canvas) canvasIDs.push_back(canvas->GetID().ToString());
    }
    activeCanvasID = activeCanvas ? activeCanvas->GetID() : UUID();
}

void UIManager::LinkScene(Scene* _scene) {
    scene = _scene;
}

ScriptManager& UIManager::GetScriptManager() const {
    if (!scene)
        throw std::runtime_error("UIManager: scene null");

    if (!scene->HasGameManager())   // add this function
        throw std::runtime_error("UIManager: gameManager null");

    return scene->GetScriptManager();
}

void UIManager::RegisterProperties() {
	SceneSystem::RegisterProperties();
	registry->Register("activeCanvas", &activeCanvasID);
	registry->Register("canvases", &canvasIDs);
}

void UIManager::SetCanvasActive(const std::string& name) {
	if (auto canvas = uiRegistry.Get(name)) {
		ActivateCanvas(canvas);
	}
}

void UIManager::ActivateCanvas(UICanvas* canvas) {
    if (activeCanvas && activeCanvas != canvas) {
        activeCanvas->DelinkManager();
    }

    activeCanvas = canvas;
    if (activeCanvas) {
        activeCanvas->SetActive(true);
        activeCanvas->LinkManager(this);
    }

    SyncCanvasIDs();
}

void UIManager::Update(float deltaTime) {
	if(activeCanvas && activeCanvas->IsActive()) {
		activeCanvas->Update(deltaTime);
	}
}

void UIManager::Draw(const glm::mat4& projection) {
	if (activeCanvas && activeCanvas->IsActive()) {
		activeCanvas->Draw(projection);
	}
}

void UIManager::HandleMouse(const glm::vec2& mousePos, bool mouseDown) {
	if (activeCanvas && activeCanvas->IsActive()) {
		activeCanvas->HandleMouse(mousePos, mouseDown);
	}
}

void UIManager::DrawImGui() {
    ImGui::Text("UI Manager");
    ImGui::SameLine();
    if (ImGui::Button("Save Canvases")) {
        uiRegistry.SaveAll();
    }
    ImGui::Separator();

    // ACTIVE CANVAS COMBO
    if (ImGui::BeginCombo("Active Canvas",
        activeCanvas ? activeCanvas->GetName().c_str() : "<none>"))
    {
        for (auto canvas : canvases)
        {
            bool selected = (canvas == activeCanvas);

            if (ImGui::Selectable(canvas->GetName().c_str(), selected))
                ActivateCanvas(canvas);

            if (selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::Separator();

    // CANVAS TABLE
    if (ImGui::BeginTable("CanvasTable", 2))
    {
        for (size_t i = 0; i < canvases.size(); i++)
        {
            UICanvas* canvas = canvases[i];

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", canvas->GetName().c_str());

            ImGui::TableSetColumnIndex(1);
            if (ImGui::SmallButton(("Remove##" + std::to_string(i)).c_str()))
            {
                if (canvas == activeCanvas) ActivateCanvas(nullptr);
                canvases.erase(canvases.begin() + i);
                SyncCanvasIDs();
                break;
            }
        }

        ImGui::EndTable();
    }

    ImGui::Separator();

    // ADD CANVAS
    if (ImGui::Button("Add Canvas"))
        ImGui::OpenPopup("AddCanvasPopup");

    if (ImGui::BeginPopup("AddCanvasPopup"))
    {
        std::vector<UICanvas*> available = uiRegistry.List();

        // ADD EXISTING
        for (UICanvas* canvas : available)
        {
            if (std::find(canvases.begin(), canvases.end(), canvas) != canvases.end()) continue;

            ImGui::PushID(canvas);
            if (ImGui::MenuItem(canvas->GetName().c_str()))
            {
                canvases.push_back(canvas);
                SyncCanvasIDs();
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopID();
        }

        // CREATE NEW
        if (ImGui::MenuItem("New Canvas"))
        {
            std::string newName =
                "Canvas_" + std::to_string(available.size());

            auto newCanvas = std::make_unique<UICanvas>(instance);
            newCanvas->SetName(newName);

            UICanvas* ptr = newCanvas.get();

            uiRegistry.Register(std::move(newCanvas));

            canvases.push_back(ptr);
            SyncCanvasIDs();

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::Separator();

    // INSPECTOR
    if (activeCanvas)
    {
        if (ImGui::CollapsingHeader("Canvas Inspector"))
        {
            activeCanvas->DrawImGui();
        }
    }
}

void UIManager::Reset() {
	activeCanvasID = UUID();
	activeCanvas = nullptr;
}
