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

    void GrabCanvasNames();
	void SetCanvasActive(const std::string&);

	void Update(float) override;
	void Draw(const glm::mat4&) override;
	void HandleMouse(const glm::vec2&, bool);
	void DrawImGui() override;

	void Reset() override;
	std::unique_ptr<SceneSystem> Clone() const override;
private:
	DelusiveUIRegistry uiRegistry;
	UICanvas* activeCanvas = nullptr;
	std::string activeCanvasName;
	std::vector<std::string> canvasList;
    std::vector<UICanvas*> canvases;
};