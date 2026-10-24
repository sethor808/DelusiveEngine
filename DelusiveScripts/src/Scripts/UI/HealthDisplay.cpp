#include <Scripts/UI/HealthDisplay.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Talismans/Talisman.h>

HealthDisplay::HealthDisplay()
	: UIScript() {
	RegisterProperties();
}

void HealthDisplay::RegisterProperties() {
	UIScript::RegisterProperties();

	registry->Register("spacing", &spacing);
	registry->Register("iconSize", &style.iconSize);
	registry->Register("stringSize", &style.stringSize);
	registry->Register("stringSpacing", &style.stringSpacing);
	registry->Register("stringXOffset", &style.stringXOffset);
	registry->Register("stringYOffset", &style.stringYOffset);
}

void HealthDisplay::RelocateReferences() {
	UIScript::RelocateReferences();
	player = nullptr;
	builtFrom.clear();
}

void HealthDisplay::OnUpdate(float) {
	if (!rootElement || !rootElement->GetEnabled()) return;

	UICanvas* canvas = rootElement->GetCanvas();
	if (!player && canvas) player = canvas->FetchPlayer();
	if (!player) return;

	std::string current = Describe();
	if (current == builtFrom) return;
	builtFrom = std::move(current);

	rootElement->ClearChildren();

	//A broken talisman is gone from the display, so the rest move up
	float x = 0.0f;
	for (const auto& talisman : player->GetTalismans()) {
		if (talisman->GetIsBroken()) continue;

		BuildTalismanVisual(rootElement, talisman.get(), rootElement->GetPosition() + glm::vec2(x, 0.0f), style);
		x += spacing;
	}
}

//Everything the visuals depend on, so equal text means nothing needs rebuilding
std::string HealthDisplay::Describe() const {
	std::string text = std::to_string(rootElement->GetPosition().x) + "," + std::to_string(rootElement->GetPosition().y);

	for (const auto& talisman : player->GetTalismans()) {
		if (talisman->GetIsBroken()) continue;
		text += "|" + talisman->GetBaseTexture() + ":" + std::to_string(talisman->GetCurrentHP());
	}
	return text;
}
