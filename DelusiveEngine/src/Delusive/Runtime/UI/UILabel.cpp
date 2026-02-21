#include <Delusive/Runtime/UI/UILabel.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h> // for Renderer::DrawText
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Internal/Rendering/Font.h>

UILabel::UILabel(DelusiveRenderer& _renderer)
	: UIElement(_renderer), text("New Text"), color({1, 1, 1, 1})
{
	fontData.fontSize = 16.0f;
	Init();
}

void UILabel::Init() {
	name = "UILabel";
	fontData.fontPath = DEFAULT_FONT;
	fontData.Init(DEFAULT_TEXT_VERT, DEFAULT_TEXT_FRAG);
	fontData.SetFont(fontData.fontPath, 48.0f);

	RegisterProperties();
}

void UILabel::RegisterProperties() {
	UIElement::RegisterProperties();
	registry->Register("font", &fontData);
	registry->Register("text", &text);
	registry->Register("color", &color);
}

void UILabel::LoadFont(const std::string& ttfPath, float pixelHeight) {
	fontData.SetFont(ttfPath, pixelHeight);
	fontData.fontSize = (pixelHeight);
}

std::unique_ptr<UIElement> UILabel::Clone() const {
	auto copy = std::make_unique<UILabel>(renderer);
	copy->SetText(text);
	copy->SetPosition(position);
	copy->SetFontSize(fontData.fontSize);
	copy->SetColor(color);
	copy->SetName(name);
	copy->SetEnabled(enabled);

	// copy font settings
	copy->fontData.fontPath = fontData.fontPath;
	copy->fontData.fontSize = fontData.fontSize;
	copy->fontData.Init();

	for (const auto& child : children) {
		copy->AddChild(std::move(child->Clone()));
	}

	return copy;
}

void UILabel::Update(float deltaTime) {
	UIElement::Update(deltaTime);
}

void UILabel::Draw(const glm::mat4& projection) {
	if (!enabled) return;

	fontData.DrawText(text, position, color, projection);

	UIElement::Draw(projection);
}

const std::string UILabel::GetType() const {
	return "UILabel";
}