#include "UIImage.h"
#include "Renderer.h"
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec4.hpp>
#include <filesystem>

float uiVertices[] = {
	// pos       // tex
	-0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  1.0f, 1.0f,

	 0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.0f, 0.0f
};

UIImage::UIImage()
	: UIElement("UIImage", { 0,0 })
{
	Init();
}

UIImage::UIImage(const glm::vec2& pos, const glm::vec2& siz)
	: UIElement("UIImage", pos)
{
	SetSize(siz);
	Init();
}

void UIImage::Init() {
	name = "UIImage";
	textureData.texturePath = "assets/sprites/star.jpg";
	textureData.Init("shaders/ui.vert", "shaders/ui.frag");

	RegisterProperties();
}

void UIImage::RegisterProperties() {
	UIElement::RegisterProperties();
	registry.Register("textureData", &textureData);
}

std::unique_ptr<UIElement> UIImage::Clone() const{
	auto copy = std::make_unique<UIImage>(position, size);
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

void UIImage::SetTexturePath(const std::string& path) {
	if (textureData.texturePath == path) return;

	textureData.previousTexturePath = textureData.texturePath;
	textureData.texturePath = path;

	if (textureData.texture) {
		delete textureData.texture;
		textureData.texture = nullptr;
	}

	textureData.texture = new Texture(textureData.texturePath.c_str());

	if (!textureData.texture) {
		std::cerr << "[UIImage] Failed to load texture: " << path << std::endl;
	}
	else {
		std::cout << "[UIImage] Texture set to: " << path << std::endl;
	}
}

void UIImage::Draw(const glm::mat4& projection) {
	if (!enabled) return;

	// Reload texture if changed
	if (textureData.texturePath != textureData.previousTexturePath) {
		SetTexturePath(textureData.texturePath);
	}

	// World position and size (accounting for pixel scale)
	glm::vec2 worldPos = position / Renderer::GetPixelScale();
	glm::vec2 worldSize = size * Renderer::GetPixelScale();

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

const std::string& UIImage::GetType() const {
	static std::string type = "UIImage";
	return type;
}