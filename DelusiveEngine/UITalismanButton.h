#pragma once
#include "UIButton.h"

class Talisman;

class UITalismanButton : public UIButton {
public:
	UITalismanButton(DelusiveRenderer&);

	const std::string GetType() const override { return "UITalismanButton"; }
	std::unique_ptr<UIElement> Clone() const override;
	void Draw(const glm::mat4&) override;
	void HandleMouse(const glm::vec2&, bool) override;
	void RegisterProperties() override;

	void BindTalisman(Talisman*);
	std::function<void(Talisman*)> onClickTalisman;

private:
	Talisman* talisman = nullptr; //Non-owned
	bool showName = false;
};