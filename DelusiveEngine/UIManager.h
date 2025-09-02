#pragma once
#include "UICanvas.h"
#include "SceneSystem.h"

class UIManager : public SceneSystem {
public:
	UIManager();

	std::string GetType() const { return "UIManager"; }
	void RegisterProperties() override;

	void SetCanvasActive(const std::string&);

	void Update(float) override;
	void Draw(const glm::mat4&) override;
	void HandleMouse(const glm::vec2&, bool);
	void DrawImGui() override;

	void Reset() override;
	std::unique_ptr<SceneSystem> Clone() const override;

	void SaveToFile(std::ofstream&) const override;

	void Deserialize(std::istream&) override;
private:
	UICanvas* activeCanvas = nullptr;
	std::string activeCanvasName;
	std::vector<std::string> canvasList;
};