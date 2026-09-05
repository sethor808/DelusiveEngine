#include <Delusive/Runtime/UI/UIImage.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec4.hpp>
#include <filesystem>

UIImage::UIImage(DelusiveInstance& instance)
	: UIElement(instance)
{
	Init();
}

void UIImage::Init() {
	name = "UIImage";
	textureData.texturePath = DEFAULT_SPRITE;
	textureData.textureID = instance.renderer.GetTexture(textureData.texturePath);

	RegisterProperties();
}

void UIImage::RegisterProperties() {
	UIElement::RegisterProperties();
	registry->Register("textureData", &textureData);
}

std::unique_ptr<UIElement> UIImage::Clone() const{
	auto copy = std::make_unique<UIImage>(instance);
	copy->SetPosition(position);
	copy->SetSize(size);
	copy->SetName(name);
	copy->SetEnabled(enabled);
	copy->SetPosition(position);
	copy->SetSize(size);
	copy->SetTexturePath(textureData.texturePath);

	for (const auto& child : children) {
		copy->AddChild(std::move(child->Clone()));
	}

	return copy;
}

void UIImage::Update(float deltaTime) {
	UIElement::Update(deltaTime);
}

void UIImage::DrawImGui() {
	UIElement::DrawImGui();

	textureData.textureID = instance.renderer.GetTexture(textureData.texturePath);
}

void UIImage::SetTexturePath(const std::string& path) {
	textureData.texturePath = path;
	textureData.textureID = instance.renderer.GetTexture(textureData.texturePath);
}

void UIImage::Draw(const glm::mat4& projection) {
	if (!enabled) return;

	glm::vec2 worldPos = position * DELUSIVE_PIXEL_SCALE;
	glm::vec2 worldSize = size * DELUSIVE_PIXEL_SCALE;

	glm::mat4 model =
		glm::translate(glm::mat4(1.0f), glm::vec3(worldPos, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(worldSize, 1.0f));

	instance.renderer.Submit({
		.modelMatrix = model,
		.color = glm::vec4(1.0f), // could expose as property
		.textureID = textureData.textureID,
		.layer = 0,
		.isUI = true
		});

	for (auto& child : children) {
		child->Draw(projection);
	}
}

const std::string UIImage::GetType() const {
	return "UIImage";
}