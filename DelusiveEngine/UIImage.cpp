#include "UIImage.h"
#include "DelusiveRenderer.h"
#include "DelusiveMacros.h"
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec4.hpp>
#include <filesystem>

UIImage::UIImage(DelusiveRenderer& _renderer)
	: UIElement(_renderer)
{
	Init();
}

void UIImage::Init() {
	name = "UIImage";
	textureData.texturePath = DEFAULT_SPRITE;
	textureData.Init(DEFAULT_UI_VERT, DEFAULT_UI_FRAG);

	RegisterProperties();
}

void UIImage::RegisterProperties() {
	UIElement::RegisterProperties();
	registry.Register("textureData", &textureData);
}

std::unique_ptr<UIElement> UIImage::Clone() const{
	auto copy = std::make_unique<UIImage>(renderer);
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
	if (!textureData.texture) {
		if (textureData.texturePath != "") {
			SetTexturePath(textureData.texturePath);
		}
	}

	//Reload texture if changed
	if (textureData.texturePath != textureData.previousTexturePath) {
		SetTexturePath(textureData.texturePath);
		textureData.previousTexturePath = textureData.texturePath;
	}

	UIElement::Update(deltaTime);
}

void UIImage::DrawImGui() {
	UIElement::DrawImGui();

	//Reloading the texture here to bypass Update not being called in the editor
	if (textureData.texturePath != textureData.previousTexturePath) {
		SetTexturePath(textureData.texturePath);
		textureData.previousTexturePath = textureData.texturePath;
	}
}

void UIImage::SetTexturePath(const std::string& path) {
	textureData.texturePath = path;

	// Clean up old texture
	if (textureData.texture) {
		delete textureData.texture;
		textureData.texture = nullptr;
	}

	// Load new texture
	textureData.texture = new Texture(path.c_str());

	//TODO: Perhaps change this to load the previous texture if it doesn't load
	if (!textureData.texture) {
		std::cerr << "[UIImage] Failed to load texture: " << path << "\n";
	}
	else {
		std::cout << "[UIImage] Texture set to: " << path << "\n";
	}
}

void UIImage::Draw(const glm::mat4& projection) {
	if (!enabled) return;

	// Reload texture if changed
	if (textureData.texturePath != textureData.previousTexturePath) {
		SetTexturePath(textureData.texturePath);
	}

	// World position and size (accounting for pixel scale)
	glm::vec2 worldPos = position * DELUSIVE_PIXEL_SCALE;
	glm::vec2 worldSize = size * DELUSIVE_PIXEL_SCALE;

	glm::mat4 model =
		glm::translate(glm::mat4(1.0f), glm::vec3(worldPos, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(worldSize, 1.0f));

	glm::mat4 view = glm::mat4(1.0f); // no view transform for UI

	textureData.Draw(model, view, projection);

	// Draw children UI elements
	for (auto& child : children) {
		child->Draw(projection);
	}
}

const std::string UIImage::GetType() const {
	return "UIImage";
}

void UIImage::Deserialize(std::istream& in) {
	UIElement::Deserialize(in);

	SetTexturePath(textureData.texturePath);
}