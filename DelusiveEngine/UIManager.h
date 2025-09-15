#pragma once
#include "UICanvas.h"
#include "DelusiveUIRegistry.h"
#include "SceneSystem.h"

class UIManager : public SceneSystem {
public:
	UIManager(DelusiveRenderer&);

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

	void Serialize(std::ostream&) const override;
	void Deserialize(std::istream&) override;
private:
	DelusiveUIRegistry uiRegistry;
	UICanvas* activeCanvas = nullptr;
	std::string activeCanvasName;
	std::vector<std::string> canvasList;
};