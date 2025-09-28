#pragma once
#include "UIElement.h"

class PlayerAgent;

class UITalismanDisplay : public UIElement {
public:
	UITalismanDisplay(DelusiveRenderer&);

	const std::string GetType() const override { return "UITalismanDisplay"; }
	std::unique_ptr<UIElement> Clone() const override;

	void RegisterProperties();

	void Update(float) override;
	void Draw(const glm::mat4& proj) override;

	void LinkPlayer(PlayerAgent* _player = nullptr);
private:
	PlayerAgent* player;
	float leftOffset = 0.0f;
	float topOffset = 0.0f;
};