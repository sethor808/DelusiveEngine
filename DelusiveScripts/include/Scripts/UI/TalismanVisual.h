#pragma once
#include <glm/glm.hpp>

class Talisman;
class UIElement;

//Sizes and offsets for drawing a talisman and its strings, in UI units
struct TalismanVisualStyle {
	float iconSize = 1.0f;
	float stringSize = 1.0f;
	float stringSpacing = 1.0f;
	float stringXOffset = 1.0f;
	float stringYOffset = 1.0f;
};

//Adds the talisman's base, glyph and one image per string it has left under parent, laid out from origin
void BuildTalismanVisual(UIElement* parent, Talisman*, const glm::vec2& origin, const TalismanVisualStyle&);
