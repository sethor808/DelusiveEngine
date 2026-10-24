#include <Scripts/UI/TalismanVisual.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/UI/UIImage.h>
#include <Delusive/Runtime/Talismans/Talisman.h>

void BuildTalismanVisual(UIElement* parent, Talisman* talisman, const glm::vec2& origin, const TalismanVisualStyle& style) {
	if (!parent || !talisman) return;

	auto* base = parent->AddChild<UIImage>();
	base->SetName("TalismanBase");
	base->SetTexturePath(talisman->GetBaseTexture());
	base->SetSize({ style.iconSize, style.iconSize });
	base->SetPosition(origin);

	auto* glyph = base->AddChild<UIImage>();
	glyph->SetName("TalismanGlyph");
	glyph->SetTexturePath(talisman->GetGlyphTexture());
	glyph->SetSize({ style.iconSize, style.iconSize });
	glyph->SetPosition(origin);

	int currentHP = talisman->GetCurrentHP();
	int maxHP = talisman->GetMaxHP();
	if (currentHP <= 0 || maxHP <= 0) return;

	float stringXStart = (style.stringSpacing * 2.0f) / maxHP;

	for (int i = 0; i < currentHP; ++i) {
		auto* stringImage = base->AddChild<UIImage>();
		stringImage->SetName("TalismanString_" + std::to_string(i));
		stringImage->SetTexturePath(talisman->GetStringTexture());
		stringImage->SetSize({ style.stringSize, style.stringSize });
		stringImage->SetPosition(origin + glm::vec2(-stringXStart + style.stringSpacing * i + style.stringXOffset, style.stringYOffset));
	}
}
