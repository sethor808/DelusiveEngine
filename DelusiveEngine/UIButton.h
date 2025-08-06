#pragma once
#include "UIElement.h"
#include <functional>

class UIButton : public UIElement {
public:
	UIButton();
	UIButton(const std::string& label, const glm::vec2& position);
	std::unique_ptr<UIElement> Clone() const override;

	void SetTexture(Texture* texture);
	void SetShader(Shader* shader);

	void Update(float) override {}
	void Draw(const glm::mat4& proj) override;
	void HandleMouse(const glm::vec2& mouse, bool mouseDown) override;

	void InitMesh();
	void SetOnClick(std::function<void()> callback);
	void DrawImGui() override;

	void Serialize(std::ostream& out) const override;
	void Deserialize(std::istream& in) override;

	const std::string& GetTypeName() const override;

private:
	std::string label;
	
	bool wasDown = false;
	std::function<void()> onClick;

	std::string font = "default";
	float fontSize = 16.0f;
	glm::vec4 fontColor = glm::vec4(1.0f);
	glm::vec2 textOffset = glm::vec2(8, 8);

	Texture* texture = nullptr;
	Shader* shader = nullptr;
	GLuint VAO = 0, VBO = 0;
};