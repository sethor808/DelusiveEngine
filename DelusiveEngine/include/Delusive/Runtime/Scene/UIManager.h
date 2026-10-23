#pragma once
#include <Delusive/Runtime/Scene/SceneSystem.h>
#include <Delusive/Runtime/Player/PlayerInputState.h>
#include <string>
#include <vector>

class ScriptManager;
class UICanvas;
class DelusiveUIRegistry;

//UI states form a stack: a base state with overlays pushed on top (pause, equip).
//Every stacked state is drawn bottom first; only the top one takes input.
class UIManager : public SceneSystem {
public:
	UIManager(DelusiveInstance&);

	~UIManager();

	ScriptManager& GetScriptManager() const;
	void Init() override;

	virtual void LinkScene(Scene*) override;
	std::string GetType() const { return "UIManager"; }
	void RegisterProperties() override;

	//States
	//Clears the stack down to this state
	void SetState(const std::string&);
	void PushState(const std::string&);
	//Never pops the base state
	void PopState();
	//Name of the top state
	const std::string& GetState() const;
	//True while any stacked state pauses the game
	bool IsPaused() const;
	//Adds the state or replaces its canvases
	void SetStateCanvases(const std::string&, const std::vector<UICanvas*>&);

	void Update(float) override;
	void Draw(const glm::mat4&) override;
	void HandleMouse(const glm::vec2&, bool);
	void HandleInput(const PlayerInputState&);
	void DrawImGui() override;

	void Reset() override;
private:
	struct UIState {
		std::string name;
		std::vector<UICanvas*> canvases;
		bool pausesGame = false;
		//SDL key name that pushes or pops this state, empty for none
		std::string toggleKey;
		bool keyWasDown = false;
	};

	//Saved as parallel lists, one entry per state; canvases are space separated ids
	std::vector<std::string> stateNames;
	std::vector<std::string> stateCanvases;
	std::vector<int> statePauses;
	std::vector<std::string> stateKeys;
	std::string startState;

	std::vector<UIState> states;
	//Indices into states, base first
	std::vector<int> stack;
	//A click that changed the stack must be released before the new top sees the mouse
	bool waitForRelease = false;

	DelusiveUIRegistry* FetchRegistry() const;
	int FindState(const std::string&) const;
	//Canvas pointers are rebuilt from the saved ids in Init
	void ResolveStates();
	void SyncStates();
	//Marks stacked canvases active, the rest inactive, after the stack changes
	void StackChanged();
	//Canvases of every stacked state, or only the top one, linked to this manager
	std::vector<UICanvas*> LinkCanvases(bool topOnly);
	void DrawStateImGui(UIState&, DelusiveUIRegistry&);
};
