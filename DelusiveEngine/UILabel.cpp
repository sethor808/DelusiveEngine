#include "UILabel.h"
#include "Renderer.h" // for Renderer::DrawText

UILabel::UILabel()
	: text("New Text"), position({ 0,0 }), fontSize(10.0f), color({1, 1, 1, 1})
{

}

UILabel::UILabel(const std::string& tex, const glm::vec2& pos, float size, const glm::vec4& col)
	: text(tex), position(pos), fontSize(size), color(col) {
}

std::unique_ptr<UIElement> UILabel::Clone() const {
	auto copy = std::make_unique<UILabel>();
	copy->SetText(text);
	copy->SetPosition(position);
	copy->SetColor(color);
	copy->SetFontSize(fontSize);

	for (const auto& child : children) {
		copy->AddChild(std::move(child->Clone()));
	}
	return copy;
}

void UILabel::Update(float deltaTime) {
	// Typically empty for labels; can animate text later if needed
	for (auto& child : children) {
		child->Update(deltaTime);
	}
}

void UILabel::Draw(const glm::mat4& proj) {
	Renderer::DrawText(text, position, fontSize, color, proj);
	for (auto& child : children) {
		child->Draw(proj);
	}
}

const std::string& UILabel::GetTypeName() const {
	std::string type = "UILabel";
	return "UILabel";
}

void UILabel::Serialize(std::ostream& out) const {
	out << "UILabel " << name << "\n";
	out << "pos " << position.x << " " << position.y << "\n";
	out << "size " << size.x << " " << size.y << "\n";
	out << "---\n";
	for (auto& child : children) {
		child->Serialize(out);
	}
}

void UILabel::Deserialize(std::istream& in) {
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
