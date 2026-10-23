#pragma once
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/GameManager.h>
#include <Delusive/Runtime/Agents/AgentTypes.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/backend/imgui_impl_sdl3.h>
#include <imgui/backend/imgui_impl_opengl3.h>
#include <filesystem>

enum class EditorMode {
	SceneEditor,
	AgentEditor,
	AnimatorEditor,
	UIBuilder,
	GameView
};

class DelusiveUIRegistry;
class AnimatorEditor;
class SceneEditor;
class UICanvas;
class UIElement;

class EngineUI {
public:
	//Window names the default layouts dock
	static constexpr const char* AgentPanelWindow = "Agent###AgentEditorPanel";
	static constexpr const char* UIBuilderPanelWindow = "UIBuilderPanel";
	EngineUI(GameManager&);
	~EngineUI();
	std::vector<std::string> LoadSceneList();
    void MoveEditorCameraTo(Agent* agent);
	void SetRenderer(const DelusiveRenderer&);

	void Render(Scene& scene);
	void RenderTopBar(Scene& scene);
	void RenderSceneEditor(Scene& scene);
	void RenderAgentEditor(Scene& scene);
	void RenderAnimatorEditor(Scene& scene);
	void RenderUIBuilder(Scene& scene);
	void RenderGameView(Scene& scene);

    void LinkEditorCamera(CameraAgent* cam);
    //Switches mode and opens an asset as if picked from the asset list
    void StartIn(Scene&, EditorMode, const std::string& asset);
private:
	GameManager& gameManager;
	DelusiveInstance& instance;
    CameraAgent* editorCamera;

	EditorMode currentMode = EditorMode::SceneEditor;
	std::string selectedAsset = "None";
	std::vector<std::string> loadedAssets;
	AgentType selectedAgentType = AgentType::None;

    //General containers
    char assetNameBuffer[64] = "";
    char renameBuffer[64] = "";
    char descBuffer[256] = "";
    std::string assetToDelete = "";

    //Popup flags
    bool openRenamePopup = false;
    bool newAssetPopup = false;
    bool showDeleteConfirm = false;

	//Scene mode lives in its own class
	std::unique_ptr<SceneEditor> sceneEditor;

	//Agent editor specifics
	bool isDraggingCollider = false;

	Component* selectedComponent = nullptr;
	bool agentSelected = true;

	//Animator mode lives in its own class
	std::unique_ptr<AnimatorEditor> animatorEditor;

	//UI builder specifics
	std::unique_ptr<DelusiveUIRegistry> uiRegistry;
	UICanvas* editingCanvas = nullptr;
	UIElement* selectedUIElement = nullptr;
	char uiCanvasNameBuffer[64] = "";

	void DrawUIElementNode(UIElement* element);

	//Helper functions
	//Each mode has its own dock space, so each keeps its own arrangement of panels
	void DockSpace();
	void BuildDefaultLayout(ImGuiID dockID);
	//Leaves play mode, if playing - pickers and asset switches act on the scene being edited
	void StopPlaying(Scene&);
	void Shortcuts(Scene&);
	bool CanUndo() const;
	bool CanRedo() const;
	void Undo(Scene&);
	void Redo(Scene&);
	void SwitchMode(Scene&, EditorMode);
	std::string GetPath(std::string);
	//Replaces the agent being edited with the one in this file
	void LoadAgentAsset(Scene&, const std::string& path);
	//Opens an asset of the current mode by name - the asset list and StartIn share it
	void OpenAsset(Scene&, const std::string& asset);
};