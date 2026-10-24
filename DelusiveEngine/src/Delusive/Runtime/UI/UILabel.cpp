#include <Delusive/Runtime/UI/UILabel.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h> // for Renderer::DrawText
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Internal/Rendering/Font.h>

UILabel::UILabel(DelusiveInstance& instance)
	: UIElement(instance), text("New Text"), color({1, 1, 1, 1})
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

void UILabel::Deserialize(DelusiveParser::DataBlock& in) {
	UIElement::Deserialize(in);

	//Init loaded the default font before the saved one arrived
	if (!fontData.fontPath.empty()) {
		fontData.SetFont(fontData.fontPath, 48.0f);
	}
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