#pragma once
#include "Scene.h"
#include "PlayerAgent.h"
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
	void Render(Scene& scene);
	void RenderTopBar(Scene& scene);
	void RenderSceneEditor(Scene& scene);
	void RenderAgentEditor(Scene& scene);
	void RenderAnimatorEditor(Scene& scene);
	void RenderGameView(Scene& scene);
private:
	int selectedAgentIndex = -1;
	EditorMode currentMode = EditorMode::SceneEditor;
	std::string selectedAsset = "None";
	std::vector<std::string> loadedAssets;
	//TODO: Define as macros somewhere
	const std::string scenePath = "assets/scenes/";
	const std::string sceneExtension = ".scene";
    const std::string agentPath = "assets/agents/";
	const std::string agentExtension = ".agent";
};