#include <Delusive/Runtime/Scene/UIManager.h>
#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <iostream>
#include <imgui/imgui.h>
#include <fstream>
#include <sstream>

UIManager::UIManager(DelusiveRenderer& _renderer) 
	: SceneSystem(_renderer), uiRegistry(_renderer)
{
	name = "NewUIManager";
	activeCanvasName = "";
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
    uiRegistry.LoadAll();
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
	registry->Register("activeCanvasName", &activeCanvasName);
	registry->Register("canvasList", &canvasList);
}

void UIManager::SetCanvasActive(const std::string& name) {
    if (activeCanvas) {
        activeCanvas->DelinkManager();
    }

	if (auto canvas = uiRegistry.Get(name)) {
		canvas->SetActive(true);
		activeCanvasName = name;
		activeCanvas = canvas;
        activeCanvas->LinkManager(this);
	}
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
    ImGui::Separator();

    // ACTIVE CANVAS COMBO
    if (ImGui::BeginCombo("Active Canvas",
        activeCanvas ? activeCanvas->GetName().c_str() : "<none>"))
    {
        for (auto canvas : canvases)
        {
            bool selected = (canvas == activeCanvas);

            if (ImGui::Selectable(canvas->GetName().c_str(), selected))
                activeCanvas = canvas;

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
                canvases.erase(canvases.begin() + i);
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
        auto names = uiRegistry.GetAllNames();

        // ADD EXISTING
        for (auto& name : names)
        {
            if (ImGui::MenuItem(name.c_str()))
            {
                if (auto canvas = uiRegistry.Get(name))
                    canvases.push_back(canvas);

                ImGui::CloseCurrentPopup();
            }
        }

        // CREATE NEW
        if (ImGui::MenuItem("New Canvas"))
        {
            std::string newName =
                "Canvas_" + std::to_string(names.size());

            auto newCanvas = std::make_unique<UICanvas>(renderer);
            newCanvas->SetName(newName);

            UICanvas* ptr = newCanvas.get();

            uiRegistry.Register(std::move(newCanvas));

            canvases.push_back(ptr);

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
	activeCanvasName.clear();
	activeCanvas = nullptr;
}

std::unique_ptr<SceneSystem> UIManager::Clone() const {
	auto clone = std::make_unique<UIManager>(renderer);
	clone->activeCanvasName = activeCanvasName;
	clone->canvasList = canvasList;

	// only refresh pointer if canvas still exists
	if (auto canvas = uiRegistry.Get(activeCanvasName)) {
		clone->activeCanvas = canvas;
	}
	return clone;
}

void UIManager::SaveToFile(std::ofstream& out) const {
    uiRegistry.SaveAll();

    // sync runtime pointers serialized names
    const_cast<UIManager*>(this)->GrabCanvasNames();

    out << "[UIManager]\n";

    out << "ActiveCanvas=" << activeCanvasName << "\n";

    out << "CanvasList=";
    for (size_t i = 0; i < canvasList.size(); ++i) {
        out << canvasList[i];
        if (i < canvasList.size() - 1) out << ",";
    }

    out << "\n";
    out << "[/UIManager]\n";
}

void UIManager::GrabCanvasNames() {
    canvasList.clear();
    for (auto* canvas : canvases) {
        if (canvas) {
            canvasList.push_back(canvas->GetName());
        }
    }
}

void UIManager::Serialize(std::ostream& out) const {
    SceneSystem::Serialize(out);
}

void UIManager::Deserialize(std::istream& in) {
	canvasList.clear();
    canvases.clear();
	activeCanvasName.clear();

	SceneSystem::Deserialize(in);

    // rebuild pointer list
    for (const auto& name : canvasList) {
        if (auto canvas = uiRegistry.Get(name)) {
            canvases.push_back(canvas);
        }
    }

	if (!activeCanvasName.empty()) {
		SetCanvasActive(activeCanvasName);
	}
}