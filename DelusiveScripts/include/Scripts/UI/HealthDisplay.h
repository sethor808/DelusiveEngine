#pragma once
#include <Delusive/Scripting/UIScript.h>
#include <Scripts/UI/TalismanVisual.h>
#include <string>

class PlayerAgent;

//Shows the player's unbroken talismans and their strings, starting at the container's position
//or, when anchored, the screen's top left corner. Rebuilds only when what it shows changes.
class HealthDisplay : public UIScript {
public:
	HealthDisplay();
	void RegisterProperties() override;

	std::string GetType() const override { return "HealthDisplay"; }
	void RelocateReferences() override;

	void OnUpdate(float) override;
private:
	//Gap between talismans
	float spacing = 1.0f;
	//Pinned to the corner of the window, offset in pixels, instead of the container's position
	bool anchorTopLeft = false;
	float leftOffset = 32.0f;
	float topOffset = 32.0f;
	TalismanVisualStyle style;

	PlayerAgent* player = nullptr;
	std::string builtFrom;

	glm::vec2 FetchOrigin() const;
	std::string Describe(const glm::vec2&) const;
};
