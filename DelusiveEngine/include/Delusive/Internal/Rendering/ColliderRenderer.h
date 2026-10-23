#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <cstddef>

class ColliderComponent;
class Shader;

class ColliderRenderer {
public:
	ColliderRenderer();
	ColliderRenderer(const ColliderRenderer&) = delete;
	ColliderRenderer& operator=(const ColliderRenderer&) = delete;
	~ColliderRenderer();
	void Draw(const ColliderComponent&, const glm::mat4&) const;
	void DrawBox(const ColliderComponent&, const glm::mat4&) const;
	void DrawCircle(const ColliderComponent&, const glm::mat4&) const;
	void DrawLine(const ColliderComponent&, const glm::mat4&) const;
	void DrawHandle(const glm::vec2& center, const glm::mat4& projection) const;
private:
	GLuint VAO, VBO;
	//Circle and line points, refilled per draw
	GLuint pointsVAO, pointsVBO;
	void DrawPoints(const glm::vec2* points, std::size_t count, GLenum mode, const glm::mat4& projection) const;
	Shader* shader;
	float handleSize = 12.0f;
};