#pragma once
#include "UICanvas.h"
#include "SceneSystem.h"

class UIManager : public SceneSystem {
public:
	UIManager();

	void SetCanvasActive(const std::string&);

	void Update(float) override;
	void Draw(const glm::mat4&) override;
	void HandleMouse(const glm::vec2&, bool);
	void DrawImGui() override;

	void Reset() override;
	std::unique_ptr<SceneSystem> Clone() const override;

	void Serialize(std::ostream&) const override;
	void Deserialize(std::istream&) override;
private:
	std::vector<std::unique_ptr<UICanvas>> canvases;
};