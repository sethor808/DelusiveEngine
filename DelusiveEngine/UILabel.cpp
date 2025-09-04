#include "UILabel.h"
#include "Renderer.h" // for Renderer::DrawText
#include "Font.h"

UILabel::UILabel()
	: text("New Text"), position({ 0,0 }), fontSize(10.0f), color({1, 1, 1, 1})
{
	name = "UILabel";
	Init();
}

UILabel::UILabel(std::string tex)
	: text(tex), position({ 0,0 }), fontSize(10.0f), color({ 1, 1, 1, 1 }) 
{
	size = { 1.0f, 1.0f };
	Init();
}

UILabel::UILabel(const std::string& tex, const glm::vec2& pos, float size, const glm::vec4& col)
	: text(tex), position(pos), fontSize(size), color(col) {
	Init();
}

UILabel::~UILabel() {
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	delete shader;
}

void UILabel::Init() {
	fontPath = "assets/fonts/pixel_arial_11/PIXEARG_.TTF";

	float verts[] = {
		// x     y      u     v
		-0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  1.0f, 1.0f,

		-0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);

	if (!shader) {
		shader = new Shader("shaders/text.vert", "shaders/text.frag");
		shader->Use();
		GLint loc = glGetUniformLocation(shader->GetID(), "tex");
		if (loc != -1) {
			glUniform1i(loc, 0);
		}
	}

	font = std::make_unique<Font>();
	loadedPixelHeight = 48.0f;
	if (!font->LoadFromFile(fontPath, loadedPixelHeight)) {
		std::cerr << "[UILabel] failed to load default font." << std::endl;
	}
}

bool UILabel::LoadFont(const std::string& ttfPath, float pixelHeight) {
	auto tmp = std::make_unique<Font>();
	if (!tmp->LoadFromFile(ttfPath, pixelHeight)) {
		return false;
	}
	font = std::move(tmp);
	loadedPixelHeight = pixelHeight;
}

std::unique_ptr<UIElement> UILabel::Clone() const {
	auto copy = std::make_unique<UILabel>();
	copy->SetText(text);
	copy->SetPosition(position);
	copy->SetColor(color);
	copy->SetFontSize(fontSize);

	copy->loadedPixelHeight = loadedPixelHeight;
	copy->LoadFont(fontPath, loadedPixelHeight);

	for (const auto& child : children) {
		copy->AddChild(std::move(child->Clone()));
	}
	return copy;
}

void UILabel::Update(float deltaTime) {
	for (auto& child : children) {
		child->Update(deltaTime);
	}
}

void UILabel::Draw(const glm::mat4& projection) {
	if (!shader || !enabled || !font) return;

	shader->Use();
	shader->SetMat4("projection", glm::value_ptr(projection));
	shader->SetVec4("uColor", color);

	glm::mat4 model(1.0f);
	shader->SetMat4("model", glm::value_ptr(model));

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	float x = position.x;
	float y = position.y;
	const float s = (loadedPixelHeight > 0.0f) ? (fontSize / loadedPixelHeight) : 1.0f;

	for (char c : text) {
		const Character& ch = font->GetCharacter(c);
		if (ch.textureID == 0 && ch.size.x == 0 && ch.size.y == 0) continue;

		float xpos = x + ch.bearing.x * s;
		float ypos = y - (ch.size.y - ch.bearing.y) * s;

		float w = ch.size.x * s;
		float h = ch.size.y * s;

		float verts[6][4] = {
			{ xpos,     ypos,     0.0f, 1.0f }, // bottom-left
			{ xpos,     ypos+h,   0.0f, 0.0f }, // top-left
			{ xpos+w,   ypos+h,   1.0f, 0.0f }, // top-right

			{ xpos,     ypos,     0.0f, 1.0f }, // bottom-left
			{ xpos+w,   ypos+h,   1.0f, 0.0f }, // top-right
			{ xpos+w,   ypos,     1.0f, 1.0f }  // bottom-right
		};

		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += ch.advance * s;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	for (auto& child : children) {
		child->Draw(projection);
	}
}

const std::string& UILabel::GetType() const {
	std::string type = "UILabel";
	return type;
}