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
#include <functional>

enum class EditorMode {
	SceneEditor,
	AgentEditor,
	AnimatorEditor,
	UIBuilder,
	GameView
};

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
	//Main window close - true when nothing is unsaved, otherwise asks first and approves later
	bool RequestClose();
	bool CloseApproved() const { return closeApproved; }
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

	//Agent editor state as last saved, to tell when it has changed
	std::vector<DelusiveParser::DataBlock> agentSaved;

	//Unsaved-changes prompt - what to do once the user saves or drops the changes
	std::function<void(Scene&)> pendingAction;
	bool unsavedPrompt = false;
	bool closeApproved = false;

	//Animator mode lives in its own class
	std::unique_ptr<AnimatorEditor> animatorEditor;

	//UI builder specifics - the scene holds a preview player and a UIManager showing the canvas
	UICanvas* editingCanvas = nullptr;
	UIElement* selectedUIElement = nullptr;
	//Deletion waits until the hierarchy has been drawn
	UIElement* uiDeleteTarget = nullptr;
	UIElement* uiDeleteParent = nullptr;
	//Hiding the panel uncovers the canvas behind it
	bool showUIBuilderPanel = true;

	void DrawUIElementNode(UIElement* element, UIElement* parent);
	void DeleteUIElement();
	void BuildUIPreview(Scene&);
	void OpenCanvas(Scene&, UICanvas*);

	//Helper functions
	//Each mode has its own dock space, so each keeps its own arrangement of panels
	void DockSpace();
	void BuildDefaultLayout(ImGuiID dockID);
	//Leaves play mode, if playing - pickers and asset switches act on the scene being edited
	void StopPlaying(Scene&);
	void Shortcuts(Scene&);
	bool HasOpenAsset() const;
	bool HasUnsavedChanges();
	void MarkAgentSaved(Scene&);
	bool SaveCurrent(Scene&);
	//Runs the action now, or after the unsaved-changes prompt
	void Guarded(std::function<void(Scene&)>);
	void UnsavedPrompt();
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