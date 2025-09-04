#include "UIButton.h"
#include "Renderer.h"
#include <imgui/imgui.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

UIButton::UIButton() 
	: UIElement("UIButton", { 0,0 }), label("New Button")
{

}

UIButton::UIButton(const std::string& label, const glm::vec2& pos)
	: UIElement(label, pos), label(label) {
	InitMesh();
}

std::unique_ptr<UIElement> UIButton::Clone() const {
	auto copy = std::make_unique<UIButton>();
	copy->SetPosition(position);
	copy->SetSize(size);
	copy->SetShader(shader);
	copy->SetTexture(texture);

	copy->label = label;
	copy->font = font;
	copy->fontSize = fontSize;
	copy->fontColor = fontColor;
	copy->textOffset = textOffset;
	copy->onClick = onClick; // WARNING: copying lambdas may not always work as expected

	for (const auto& child : children) {
		copy->AddChild(std::move(child->Clone()));
	}
	return copy;
}

void UIButton::SetTexture(Texture* tex) {
	texture = tex;
}

void UIButton::SetShader(Shader* s) {
	shader = s;
}

void UIButton::SetOnClick(std::function<void()> callback) {
	onClick = std::move(callback);
}

void UIButton::Draw(const glm::mat4& projection) {
	if (!enabled || !shader) return;

	shader->Use();
	glActiveTexture(GL_TEXTURE0);
	texture->Bind();
	glUniform1i(glGetUniformLocation(shader->GetID(), "tex"), 0);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));

	shader->SetMat4("model", glm::value_ptr(model));
	shader->SetMat4("projection", glm::value_ptr(projection));
	shader->SetMat4("view", glm::value_ptr(glm::mat4(1.0f)));

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	Renderer::DrawText(label, position + glm::vec2(8, 8), 16.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, projection);

	for (auto& child : children) {
		child->Draw(projection);
	}
}

void UIButton::InitMesh() {
	float vertices[] = {
		// pos     // tex
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // Position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1); // TexCoord
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);
}

void UIButton::HandleMouse(const glm::vec2& mouse, bool mouseDown) {
	bool inside = mouse.x >= position.x && mouse.x <= position.x + size.x &&
		mouse.y >= position.y && mouse.y <= position.y + size.y;

	if (inside && !wasDown && mouseDown && onClick) {
		onClick();
	}
	wasDown = mouseDown;
}

const std::string& UIButton::GetType() const {
	static std::string type = "UIButton";
	return type;
}