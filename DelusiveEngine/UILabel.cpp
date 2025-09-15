#include "UILabel.h"
#include "Renderer.h" // for Renderer::DrawText
#include "Font.h"

UILabel::UILabel()
	: text("New Text"), color({1, 1, 1, 1})
{
	fontData.fontSize = 16.0f;
	Init();
}

UILabel::UILabel(std::string tex)
	: text(tex), color({ 1, 1, 1, 1 }) 
{
	size = { 1.0f, 1.0f };
	fontData.fontSize = 16.0f;
	Init();
}

UILabel::UILabel(const std::string& tex, const glm::vec2& pos, float size, const glm::vec4& col)
	: text(tex), color(col) {
	fontData.fontSize = size;
	Init();
}

void UILabel::Init() {
	name = "UILabel";
	fontData.fontPath = "assets/fonts/pixel_arial_11/PIXEARG_.TTF";
	fontData.Init("shaders/text.vert", "shaders/text.frag");
	fontData.SetFont(fontData.fontPath, 48.0f);

	RegisterProperties();
}

void UILabel::RegisterProperties() {
	UIElement::RegisterProperties();
	registry.Register("font", &fontData);
	registry.Register("text", &text);
	registry.Register("color", &color);
}

void UILabel::LoadFont(const std::string& ttfPath, float pixelHeight) {
	fontData.SetFont(ttfPath, pixelHeight);
	fontData.fontSize = (pixelHeight);
}

std::unique_ptr<UIElement> UILabel::Clone() const {
	auto copy = std::make_unique<UILabel>(text, position, fontData.fontSize, color);
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