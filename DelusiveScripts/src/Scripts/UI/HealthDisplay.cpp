#include <Scripts/UI/HealthDisplay.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/UI/UIScriptContainer.h>
#include <Delusive/Runtime/UI/UICanvas.h>
#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Runtime/Talismans/Talisman.h>

HealthDisplay::HealthDisplay()
	: UIScript() {
	RegisterProperties();
}

void HealthDisplay::RegisterProperties() {
	UIScript::RegisterProperties();

	registry->Register("spacing", &spacing);
	registry->Register("anchorTopLeft", &anchorTopLeft);
	registry->Register("leftOffset", &leftOffset);
	registry->Register("topOffset", &topOffset);
	registry->Register("iconSize", &style.iconSize);
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

	const glm::vec2 origin = FetchOrigin();
	std::string current = Describe(origin);
	if (current == builtFrom) return;
	builtFrom = std::move(current);

	rootElement->ClearChildren();

	//A broken talisman is gone from the display, so the rest move up
	float x = 0.0f;
	for (const auto& talisman : player->GetTalismans()) {
		if (talisman->GetIsBroken()) continue;

		BuildTalismanVisual(rootElement, talisman.get(), origin + glm::vec2(x, 0.0f), style);
		x += spacing;
	}
}

glm::vec2 HealthDisplay::FetchOrigin() const {
	if (!anchorTopLeft) return rootElement->GetPosition();

	int windowW, windowH;
	rootElement->GetInstance().renderer.GetWindowSize(windowW, windowH);
	const glm::vec2 corner = { (-windowW / 2.0f + leftOffset) / DELUSIVE_PIXEL_SCALE, (windowH / 2.0f - topOffset) / DELUSIVE_PIXEL_SCALE };
	//Talismans are drawn centred, so step in by half of one to keep the first fully on screen
	return corner + glm::vec2(style.iconSize, -style.iconSize) * 0.5f;
}

//Everything the visuals depend on, so equal text means nothing needs rebuilding
std::string HealthDisplay::Describe(const glm::vec2& origin) const {
	std::string text = std::to_string(origin.x) + "," + std::to_string(origin.y);

	for (const auto& talisman : player->GetTalismans()) {
		if (talisman->GetIsBroken()) continue;
		text += "|" + talisman->GetBaseTexture() + ":" + std::to_string(talisman->GetCurrentHP());
	}
	return text;
}
