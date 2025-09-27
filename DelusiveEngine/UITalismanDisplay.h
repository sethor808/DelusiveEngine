#pragma once
#include "UIElement.h"

class PlayerAgent;

class UITalismanDisplay : public UIElement {
public:
	UITalismanDisplay(DelusiveRenderer&);

	const std::string GetType() const override { return "UITalismanDisplay"; }
	std::unique_ptr<UIElement> Clone() const;

	void Update(float) override;
	void Draw(const glm::mat4& proj) override;

	void LinkPlayer(PlayerAgent* _player = nullptr);
private:
	PlayerAgent* player;
};