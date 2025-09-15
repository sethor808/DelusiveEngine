#include "DelusiveRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "DelusiveMacros.h"

DelusiveRenderer::~DelusiveRenderer() {
	Shutdown();
}

void DelusiveRenderer::Init() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	OnResize(width, height);

	float quadVertices[] = {
		// positions   // texCoords
		0.0f, 1.0f,    0.0f, 1.0f,
		1.0f, 0.0f,    1.0f, 0.0f,
		0.0f, 0.0f,    0.0f, 0.0f,

		0.0f, 1.0f,    0.0f, 1.0f,
		1.0f, 1.0f,    1.0f, 1.0f,
		1.0f, 0.0f,    1.0f, 0.0f
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);

	InitTextRenderer();
}

void DelusiveRenderer::Clear() {
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

GLuint DelusiveRenderer::CreateFallbackWhiteTexture() {
	static GLuint fallbackTex = 0;
	if (fallbackTex == 0) {
		std::cout << "Creating tex" <<std::endl;
		glGenTextures(1, &fallbackTex);
		glBindTexture(GL_TEXTURE_2D, fallbackTex);
		uint8_t whitePixel[4] = { 255, 255, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	return fallbackTex;
}

void DelusiveRenderer::GenerateProjection() {
	float halfWidthUnits = width / (2.0f * DELUSIVE_PIXEL_SCALE);
	float halfHeightUnits = height / (2.0f * DELUSIVE_PIXEL_SCALE);
	projection = glm::ortho(
		-halfWidthUnits, halfWidthUnits,
		-halfHeightUnits, halfHeightUnits,
		-1.0f, 1.0f
	);
}

void DelusiveRenderer::GetWindowSize(int& _width, int& _height) {
	_width = width;
	_height = height;
}

void DelusiveRenderer::OnResize(int _width, int _height) {
	if (width != _width || height != _height) {
		width = _width;
		height = _height;
		glViewport(0, 0, _width, _height);
	}
	
	GenerateProjection();
}

float DelusiveRenderer::GetPixelScale() {
	return DELUSIVE_PIXEL_SCALE;
}

void DelusiveRenderer::Shutdown() {
	if (quadVBO) glDeleteBuffers(1, &quadVBO);
	if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
	quadVBO = quadVAO = 0;

	if (textVBO) glDeleteBuffers(1, &textVBO);
	if (textVAO) glDeleteVertexArrays(1, &textVAO);
	textVBO = textVAO = 0;
}

const glm::mat4& DelusiveRenderer::GetProjection() const{
	return projection;
}

glm::mat4 DelusiveRenderer::GetUIProjection() {
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.0f, 1.0f);
}

void DelusiveRenderer::BeginUIRenderPass() {
	glDisable(GL_DEPTH_TEST);
}

void DelusiveRenderer::EndUIRenderPass() {
	glEnable(GL_DEPTH_TEST);
}

void DelusiveRenderer::DebugDrawLine(glm::vec2 a, glm::vec2 b, glm::vec4 color) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&projection[0][0]);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);
	glColor4f(color.r, color.g, color.b, color.a);

	glBegin(GL_LINES);
	glVertex2f(a.x, a.y);
	glVertex2f(b.x, b.y);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void DelusiveRenderer::DebugDrawRect(glm::vec2 center, float size, glm::vec4 color) {
	float halfSize = size * 0.5f;
	glm::vec2 topLeft = center + glm::vec2(-halfSize, halfSize);
	glm::vec2 topRight = center + glm::vec2(halfSize, halfSize);
	glm::vec2 bottomRight = center + glm::vec2(halfSize, -halfSize);
	glm::vec2 bottomLeft = center + glm::vec2(-halfSize, -halfSize);

	DebugDrawLine(topLeft, topRight, color);
	DebugDrawLine(topRight, bottomRight, color);
	DebugDrawLine(bottomRight, bottomLeft, color);
	DebugDrawLine(bottomLeft, topLeft, color);
}

void DelusiveRenderer::InitTextRenderer() {
	// Generate VAO/VBO
	glGenVertexArrays(1, &textVAO);
	glGenBuffers(1, &textVBO);

	glBindVertexArray(textVAO);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Load text shader (managed by unique_ptr)
	textShader = std::make_unique<Shader>(DEFAULT_TEXT_VERT, DEFAULT_TEXT_FRAG);
	textShader->Use();
	textShader->SetInt("text", 0); // instead of raw glUniform1i

	// Load default font (also managed by unique_ptr)
	defaultFont = std::make_unique<Font>();
	if (!defaultFont->LoadFromFile(DEFAULT_FONT, 48)) {
		std::cerr << "Failed to load default font!\n";
	}
}

void DelusiveRenderer::DrawText(const std::string& text, glm::vec2 position, float size, glm::vec4 color, const glm::mat4& proj) {
	if (!defaultFont || !textShader) return;

	float scale = size;
	float x = position.x;
	float y = position.y;

	glActiveTexture(GL_TEXTURE0);
	textShader->Use();
	textShader->SetMat4("uProjection", proj);
	textShader->SetVec4("uColor", color);

	glBindVertexArray(textVAO);

	for (char c : text) {
		const Character& ch = defaultFont->GetCharacter(c);
		float xpos = x + ch.bearing.x * scale;
		float ypos = y - (ch.size.y - ch.bearing.y) * scale;

		float w = ch.size.x * scale;
		float h = ch.size.y * scale;

		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		x += (ch.advance >> 6) * scale; // Advance is in 1/64 pixels
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

Shader* DelusiveRenderer::GetDefaultUIShader() {

	if (!uiShader.get()) {
		uiShader = std::make_unique<Shader>(
			"../assets/shaders/ui.vert",
			"../assets/shaders/ui.frag"
		);
	}

	return uiShader.get();
}

void DelusiveRenderer::DrawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color,
	const glm::mat4& projection, Shader* shader, GLuint texture) {

	Shader* usedShader = shader ? shader : uiShader.get();
	usedShader->Use();
	usedShader->SetMat4("uProjection", projection);
	usedShader->SetVec2("uPos", pos);
	usedShader->SetVec2("uSize", size);
	usedShader->SetVec4("uColor", color);
	usedShader->SetInt("uTexture", 0);

	if (texture != 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}