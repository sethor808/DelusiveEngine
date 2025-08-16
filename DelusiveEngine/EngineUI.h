#pragma once
#include "Scene.h"
#include "PlayerAgent.h"
#include "AnimatorData.h"
#include "Animation.h"
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

class EngineUI {
public:
	EngineUI();
	~EngineUI();
	std::vector<std::string> LoadSceneList();
	ImTextureID GetFramePreviewTexture(AnimationFrame&, Agent&);
	void Render(Scene& scene);
	void RenderTopBar(Scene& scene);
	void RenderSceneEditor(Scene& scene);
	void RenderAgentEditor(Scene& scene);
	void RenderAnimatorEditor(Scene& scene);
	void RenderGameView(Scene& scene);
private:
	int selectedAgentIndex = -1;
	int selectedSystemIndex = -1;
	EditorMode currentMode = EditorMode::SceneEditor;
	std::string selectedAsset = "None";
	std::vector<std::string> loadedAssets;
	AgentType selectedAgentType = AgentType::None;

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

	//TODO: Define as macros somewhere
	const std::string scenePath = "assets/scenes/";
	const std::string sceneExtension = ".scene";
    const std::string agentPath = "assets/agents/";
	const std::string agentExtension = ".agent";
	const std::string animationPath = "assets/animations/";
	const std::string animationExtension = ".anim";

	//Helper functions
	void SwitchMode(Scene&, EditorMode);
	std::string GetPath(std::string);
	void ClearFramePreviews(Animation& anim);
	void ApplyOverrides(AnimationFrame&, Agent&);
	void ResetOverrides();
	void SetupAnimation(const std::string);
	void RenderAnimationOverrides(AnimationFrame&, ComponentMod&);
};