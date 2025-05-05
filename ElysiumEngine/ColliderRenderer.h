#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "Collider.h"

class ColliderRenderer {
public:
	ColliderRenderer();
	~ColliderRenderer();
	void Draw(const Collider& collider, const glm::mat4& projection) const;
	void DrawCenterHandle(const glm::vec2& center, const glm::mat4& projection) const;

private:
	GLuint VAO, VBO;
	Shader* shader;
};