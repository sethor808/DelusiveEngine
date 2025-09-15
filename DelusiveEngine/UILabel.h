#pragma once
#include "UIElement.h"
#include <string>
#include <glm/glm.hpp>
#include "Font.h"

class UILabel : public UIElement {
public:
	UILabel();
	UILabel(const std::string);
	UILabel(const std::string& text, const glm::vec2& pos, float size = 16.0f, const glm::vec4& color = { 1, 1, 1, 1 });

	void RegisterProperties() override;
	std::unique_ptr<UIElement> Clone() const override;

	void Init();
	void LoadFont(const std::string&, float);

	void Update(float deltaTime) override;
	void Draw(const glm::mat4& proj) override;

	void SetText(const std::string& newText) { text = newText; }
	const std::string& GetText() const { return text; }

	void SetPosition(const glm::vec2& pos) { position = pos; }
	glm::vec2 GetPosition() const { return position; }

	void SetColor(const glm::vec4& col) { color = col; }
	glm::vec4 GetColor() const { return color; }

	void SetFontSize(float s) { fontData.fontSize = s; }
	float GetFontSize() const { return fontData.fontSize; }

	const std::string GetType() const override;
private:
	std::string text;
	glm::vec4 color;

	DelusiveFont fontData;
};