#pragma once
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/UI/DelusiveUIRegistry.h>
#include <Delusive/Runtime/Scene/SceneSystem.h>

class ScriptManager;

class UIManager : public SceneSystem {
public:
	UIManager(DelusiveInstance&);

	~UIManager();

    ScriptManager& GetScriptManager() const;
    void Init() override;

    virtual void LinkScene(Scene*) override;
	std::string GetType() const { return "UIManager"; }
	void RegisterProperties() override;

	void SetCanvasActive(const std::string&);
	void ActivateCanvas(UICanvas*);
	UICanvas* GetActiveCanvas() const { return activeCanvas; }

	void Update(float) override;
	void Draw(const glm::mat4&) override;
	void HandleMouse(const glm::vec2&, bool);
	void DrawImGui() override;

	void Reset() override;
private:
    //Canvases are referenced by UUID on disk; the pointers are rebuilt in Init
    void ResolveCanvases();
    void SyncCanvasIDs();

	DelusiveUIRegistry uiRegistry;
	UICanvas* activeCanvas = nullptr;
	UUID activeCanvasID;
	std::vector<std::string> canvasIDs;
    std::vector<UICanvas*> canvases;
};