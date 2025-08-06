#include "UIImage.h"
#include "Renderer.h"
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

UIImage::UIImage()
	: UIElement("UIImage", { 0,0 }), size({ 1,1 })
{

}

UIImage::UIImage(const glm::vec2& pos, const glm::vec2& siz)
	: UIElement("UIImage", pos), size(siz) {
}

std::unique_ptr<UIElement> UIImage::Clone() const{
	auto copy = std::make_unique<UIImage>(position, size);
	copy->SetName(name);
	copy->SetEnabled(enabled);
	copy->SetPosition(position);
	copy->SetSize(size);
	copy->SetTexture(texture);
	copy->SetShader(shader);

	for (const auto& child : children) {
		copy->AddChild(std::move(child->Clone()));
	}

	return copy;
}

void UIImage::SetTexture(Texture* tex) {
	texture = tex;
}

void UIImage::SetShader(Shader* s) {
	shader = s;
}

void UIImage::Draw(const glm::mat4& projection) {
	if (!enabled || !texture || !shader) return;

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

	for (auto& child : children) {
		child->Draw(projection);
	}
}

void UIImage::DrawImGui() {
	ImGui::Text("UIImage");
	ImGui::DragFloat2("Position", &position.x, 1.0f);
	ImGui::DragFloat2("Size", &size.x, 1.0f);
}

void UIImage::Serialize(std::ostream& out) const {
	out << "UIImage " << name << "\n";
	out << "pos " << position.x << " " << position.y << "\n";
	out << "size " << size.x << " " << size.y << "\n";
	out << "---\n";
	for (auto& child : children) {
		child->Serialize(out);
	}
}

void UIImage::Deserialize(std::istream& in) {
	std::string token;
	while (in >> token) {
		if (token == "---") break;
		else if (token == "pos") in >> position.x >> position.y;
		else if (token == "size") in >> size.x >> size.y;
	}
	for (auto& child : children) {
		child->Deserialize(in);
	}
}

const std::string& UIImage::GetTypeName() const {
	static std::string type = "UIImage";
	return type;
}