#pragma once
#include "UIElement.h"

class UIImage : public UIElement {
public:
	UIImage();
	UIImage(const glm::vec2& pos, const glm::vec2& size);
	~UIImage();
	std::unique_ptr<UIElement> Clone() const override;

	void Init();
	void SetTexturePath(const std::string&);
	void SetShader(Shader* shader);

	void Draw(const glm::mat4& proj) override;
	void HandleMouse(const glm::vec2&, bool) override {}
	void DrawImGui() override;

	void Serialize(std::ostream& out) const override;
	void Deserialize(std::istream& in) override;

	const std::string& GetTypeName() const override;

private:
	glm::vec2 size;
	std::string texturePath = "";
	Shader* shader = nullptr;
	Texture* texture = nullptr;
	GLuint VAO, VBO;
};