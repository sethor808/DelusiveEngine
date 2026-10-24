#pragma once
#include <Delusive/Scripting/UIScript.h>
#include <Scripts/UI/TalismanVisual.h>
#include <string>

class PlayerAgent;

//Shows the player's unbroken talismans and their strings, starting at the container's position.
//Rebuilds only when the talisman stack or the container's position changes.
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
	TalismanVisualStyle style = { 0.75f, 0.15f, 0.2f, 0.0f, -0.55f };

	PlayerAgent* player = nullptr;
	std::string builtFrom;

	std::string Describe() const;
};
