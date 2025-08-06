#pragma once
#include "UIElement.h"
#include <string>
#include <glm/glm.hpp>

class UILabel : public UIElement {
public:
	UILabel();
	UILabel(const std::string& text, const glm::vec2& pos, float size = 16.0f, const glm::vec4& color = { 1, 1, 1, 1 });
	std::unique_ptr<UIElement> Clone() const override;

	void Update(float deltaTime) override;
	void Draw(const glm::mat4& proj) override;

	void SetText(const std::string& newText) { text = newText; }
	const std::string& GetText() const { return text; }

	void SetPosition(const glm::vec2& pos) { position = pos; }
	glm::vec2 GetPosition() const { return position; }

	void SetColor(const glm::vec4& col) { color = col; }
	glm::vec4 GetColor() const { return color; }

	void SetFontSize(float s) { fontSize = s; }
	float GetFontSize() const { return fontSize; }

	const std::string& GetTypeName() const override;
	void Serialize(std::ostream&) const;
	void Deserialize(std::istream&);
private:
	std::string text;
	glm::vec2 position;
	glm::vec2 size;
	glm::vec4 color;
	float fontSize;
};