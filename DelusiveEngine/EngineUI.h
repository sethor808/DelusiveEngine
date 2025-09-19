#pragma once
#include "Scene.h"
#include "AnimatorData.h"
#include "Animation.h"
#include "GameManager.h"
#include "AgentTypes.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/backend/imgui_impl_sdl3.h>
#include <imgui/backend/imgui_impl_opengl3.h>
#include <filesystem>

enum class EditorMode {
	SceneEditor,
	AgentEditor,
	AnimatorEditor,
	GameView
};

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

class EngineUI {
public:
	EngineUI(GameManager&, DelusiveRenderer&);
	~EngineUI();
	std::vector<std::string> LoadSceneList();
	ImTextureID GetFramePreviewTexture(AnimationFrame&, Agent&);
	void SetRenderer(const DelusiveRenderer&);
	void Render(Scene& scene);
	void RenderTopBar(Scene& scene);
	void RenderSceneEditor(Scene& scene);
	void RenderAgentEditor(Scene& scene);
	void RenderAnimatorEditor(Scene& scene);
	void RenderGameView(Scene& scene);
private:
	GameManager& gameManager;
	DelusiveRenderer& renderer;

	EditorMode currentMode = EditorMode::SceneEditor;
	std::string selectedAsset = "None";
	std::vector<std::string> loadedAssets;
	AgentType selectedAgentType = AgentType::None;

	//Scene editor specifics
    Selection selected;
	int agentToDeleteIndex = -1;

	//Agent editor specifics
	bool isDraggingCollider = false;

	//AnimatorData
	Animation currentAnimation;
	int selectedFrame = -1;
	int selectedBranch = -1;
	Component* selectedComponent = nullptr;
	bool agentSelected = true;
	std::unique_ptr<Agent> baseAgent = nullptr;
	std::unique_ptr<Agent> pureAgent = nullptr;
	std::string currentBaseAgentFile;
	bool confirmAgentSwitch = false;
	std::string pendingAgentFile;

	//Helper functions
	void SwitchMode(Scene&, EditorMode);
	std::string GetPath(std::string);
	void ClearFramePreviews(Animation& anim);
	void ApplyOverrides(AnimationFrame&, Agent&);
	void ResetOverrides();
	void SetupAnimation(const std::string);
	void RenderAnimationOverrides(AnimationFrame&, ComponentMod&);
};