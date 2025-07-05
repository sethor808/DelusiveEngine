#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

static glm::mat4 projection;
static int width = 800;
static int height = 600;

void Renderer::Init() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	OnResize(width, height);
}

void Renderer::Clear() {
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

GLuint Renderer::CreateFallbackWhiteTexture() {
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

void Renderer::GetWindowSize(int& _width, int& _height) {
	width = _width;
	height = _height;
}

void Renderer::OnResize(int _width, int _height) {
	width = _width;
	height = _height;
	glViewport(0, 0, _width, _height);
	float pixelsPerUnit = 64.0f;
	float halfWidthUnits = _width / (2.0f * pixelsPerUnit);
	float halfHeightUnits = _height / (2.0f * pixelsPerUnit);
	projection = glm::ortho(
		-halfWidthUnits, halfWidthUnits,
		-halfHeightUnits, halfHeightUnits,
		-1.0f, 1.0f
	);
}

void Renderer::Shutdown() {

}

const glm::mat4& Renderer::GetProjection() {
	return projection;
}