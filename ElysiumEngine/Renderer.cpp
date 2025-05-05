#include "Renderer.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

static glm::mat4 projection;

void Renderer::Init() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	OnResize(800, 600);
}

void Renderer::Clear() {
	glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::OnResize(int width, int height) {
	glViewport(0, 0, width, height);
	projection = glm::ortho(0.0f, float(width), float(height), 0.0f);
}

void Renderer::Shutdown() {

}

const glm::mat4& Renderer::GetProjection() {
	return projection;
}