#pragma once
#include <glm/glm.hpp>

class Talisman;
class UIElement;

//Sizes and offsets for drawing a talisman and its strings, in UI units.
//String sprites share the talisman's frame, so they are drawn at the talisman's size.
struct TalismanVisualStyle {
	float iconSize = 0.75f;
	float stringSpacing = 0.08f;
	float stringXOffset = 0.0f;
	float stringYOffset = 0.0f;
};

//Adds the talisman's base, glyph and one image per string it has left under parent, laid out from origin
void BuildTalismanVisual(UIElement* parent, Talisman*, const glm::vec2& origin, const TalismanVisualStyle&);
