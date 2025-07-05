#pragma once
#include <glm/glm.hpp>
#include "Shader.h"
#include "ColliderComponent.h"

class ColliderComponent;

class ColliderRenderer {
public:
	ColliderRenderer();
	~ColliderRenderer();
	void Draw(const ColliderComponent& collider, const glm::mat4& projection) const;
	void DrawCenterHandle(const glm::vec2& center, const glm::mat4& projection) const; //Outdated

	void DrawHandles(const ColliderComponent& collider, const glm::mat4& projection) const;
	void DrawHandle(const glm::vec2& center, const glm::mat4& projection) const;
private:
	GLuint VAO, VBO;
	Shader* shader;
};