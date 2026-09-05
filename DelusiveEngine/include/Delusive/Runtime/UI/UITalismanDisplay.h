#pragma once
#include <Delusive/Runtime/UI/UIElement.h>

class PlayerAgent;

class UITalismanDisplay : public UIElement {
public:
	UITalismanDisplay(DelusiveInstance&);

	const std::string GetType() const override { return "UITalismanDisplay"; }
	std::unique_ptr<UIElement> Clone() const override;

	void RegisterProperties();

	void Update(float) override;
	void Draw(const glm::mat4& proj) override;

	void LinkPlayer(PlayerAgent* _player = nullptr);
private:
	PlayerAgent* player;
	//Talisman settings
	float leftOffset = 0.0f;
	float topOffset = 0.0f;
	float iconSize = 32.0f;
	float spacing = 8.0f;

	//String settings
	float stringSize = 5.0f;
	float stringXOffset = 1.0f;
	float stringYOffset = 16.0f;
	float stringSpacing = 2.0f;
};