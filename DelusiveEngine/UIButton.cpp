#include "UIButton.h"
#include "DelusiveRenderer.h"
#include <imgui/imgui.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

UIButton::UIButton(DelusiveRenderer& _renderer) 
	: UIElement(_renderer)
{
	Init();
}

void UIButton::Init() {
	buttonTexture.texturePath = "../assets/ui/default_button.png";
	buttonTexture.Init();

	buttonFont.fontPath = DEFAULT_FONT;
	buttonFont.fontSize = 16;
	buttonFont.Init();

	RegisterProperties();
}

void UIButton::RegisterProperties() {
	UIElement::RegisterProperties();
	registry.Register("Label", &label);
	registry.Register("ButtonTexture", &buttonTexture);
	registry.Register("Font", &buttonFont);
	registry.Register("FontColor", &fontColor);
	registry.Register("TextOffset", &textOffset);
}

std::unique_ptr<UIElement> UIButton::Clone() const {
	auto copy = std::make_unique<UIButton>(renderer);
	//TODO: Set label
	copy->SetPosition(position);
	copy->SetSize(size);
	copy->SetEnabled(enabled);

	copy->buttonTexture.CloneFrom(buttonTexture);
	copy->buttonFont.CloneFrom(buttonFont);
	copy->fontColor = fontColor;
	copy->buttonFont.fontSize = buttonFont.fontSize;
	copy->textOffset = textOffset;
	copy->onClick = onClick; // note: lambda copying can be tricky

	for (const auto& child : children) {
		copy->AddChild(std::move(child->Clone()));
	}

	return copy;
}

void UIButton::SetOnClick(std::function<void()> callback) {
	onClick = std::move(callback);
}

void UIButton::Draw(const glm::mat4& projection) {
	if (!enabled) return;

	// draw button background
	glm::mat4 model =
		glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

	glm::mat4 view = glm::mat4(1.0f);

	buttonTexture.Draw(model, view, projection);

	// draw button label
	buttonFont.DrawText(
		label,
		position + textOffset,
		fontColor,
		projection
	);

	UIElement::Draw(projection); // draw children
}

void UIButton::HandleMouse(const glm::vec2& mouse, bool mouseDown) {
	bool inside = mouse.x >= position.x && mouse.x <= position.x + size.x &&
		mouse.y >= position.y && mouse.y <= position.y + size.y;

	if (inside && !wasDown && mouseDown && onClick) {
		onClick();
	}
	wasDown = mouseDown;
}

const std::string UIButton::GetType() const {
	return "UIButton";
}