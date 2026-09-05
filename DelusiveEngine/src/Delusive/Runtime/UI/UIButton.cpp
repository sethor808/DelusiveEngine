#include <Delusive/Runtime/UI/UIButton.h>
#include <Delusive/Runtime/Core/DelusiveCoreIncludes.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <imgui/imgui.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

UIButton::UIButton(DelusiveInstance& instance)
	: UIElement(instance)
{
	Init();
}

void UIButton::Init() {
	buttonTexture.texturePath = "../assets/ui/default_button.png";

	buttonTexture.textureID = instance.renderer.GetTexture(buttonTexture.texturePath);
	if (buttonTexture.textureID == 0) {
		buttonTexture.textureID = instance.renderer.CreateFallbackWhiteTexture();
	}

	// Font initialization (leave as-is if your Font class handles its own resources)
	buttonFont.fontPath = DEFAULT_FONT;
	buttonFont.fontSize = 16;
	buttonFont.Init();

	RegisterProperties();
}

void UIButton::RegisterProperties() {
	UIElement::RegisterProperties();
	registry->Register("Label", &label);
	registry->Register("ButtonTexture", &buttonTexture);
	registry->Register("Font", &buttonFont);
	registry->Register("FontColor", &fontColor);
	registry->Register("TextOffset", &textOffset);
}

std::unique_ptr<UIElement> UIButton::Clone() const {
	auto copy = std::make_unique<UIButton>(instance);
	copy->SetPosition(position);
	copy->SetSize(size);
	copy->SetEnabled(enabled);
	copy->SetName(name);

	// copy simple data
	copy->label = label;
	copy->fontColor = fontColor;
	copy->textOffset = textOffset;

	// copy texture path and ask renderer for the cached textureID
	copy->buttonTexture.texturePath = buttonTexture.texturePath;
	copy->buttonTexture.textureID = instance.renderer.GetTexture(copy->buttonTexture.texturePath);
	if (copy->buttonTexture.textureID == 0) {
		copy->buttonTexture.textureID = instance.renderer.CreateFallbackWhiteTexture();
	}

	// Copy font (if Font is copyable; adjust if it requires a special clone)
	copy->buttonFont.fontSize = buttonFont.fontSize;

	// copy onClick (note: copying lambdas with captures might not behave as you expect)
	copy->onClick = onClick;

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

	// Compute world-space position/size for UI (accounting for pixel scale)
	glm::vec2 worldPos = position * DELUSIVE_PIXEL_SCALE;
	glm::vec2 worldSize = size * DELUSIVE_PIXEL_SCALE;

	glm::mat4 model =
		glm::translate(glm::mat4(1.0f), glm::vec3(worldPos, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(worldSize, 1.0f));

	// Build render command and submit to the renderer (batched)
	RenderCommand cmd;
	cmd.modelMatrix = model;
	cmd.color = glm::vec4(1.0f); // could become a property/tint
	cmd.textureID = buttonTexture.textureID ? buttonTexture.textureID : instance.renderer.CreateFallbackWhiteTexture();
	cmd.layer = 0; // UI layer ordering -- feel free to expose as a member
	cmd.isUI = true;
	instance.renderer.Submit(cmd);

	// Draw label text using your font system (unchanged)
	// Note: position for text should be in same coordinate space as Draw (center-origin)
	buttonFont.DrawText(
		label,
		position + textOffset,
		fontColor,
		projection
	);

	// Draw children (they will submit their own RenderCommands)
	UIElement::Draw(projection);
}

void UIButton::SetTexturePath(const std::string& path) {

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