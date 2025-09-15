#pragma once
#include "UIElement.h"
#include <functional>

class UIButton : public UIElement {
public:
	UIButton(DelusiveRenderer&);
	UIButton(const std::string& label, const glm::vec2& position);
	std::unique_ptr<UIElement> Clone() const override;

	void Init();
	void RegisterProperties() override;

	void Update(float) override {}
	void Draw(const glm::mat4& proj) override;
	void HandleMouse(const glm::vec2& mouse, bool mouseDown) override;

	void SetOnClick(std::function<void()> callback);

	const std::string GetType() const override;
private:
	std::string label = "New Button";

	bool wasDown = false;
	std::function<void()> onClick;

	DelusiveTexture buttonTexture;
	DelusiveFont buttonFont;

	glm::vec4 fontColor = { 1, 1, 1, 1 };
	glm::vec2 textOffset = { 8, 8 };
};