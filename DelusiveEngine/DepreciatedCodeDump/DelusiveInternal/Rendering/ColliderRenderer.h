#pragma once
#include <glm/glm.hpp>
#include <DelusiveInternal/Rendering/Shader.h>
#include <DelusiveInternal/Components/ColliderComponent.h>
#include <DelusiveInternal/Rendering/DelusiveRenderer.h>

class ColliderComponent;

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
	void DrawCenterHandle(const glm::vec2&, const glm::mat4&) const; //Outdated

	void DrawHandles(const ColliderComponent&, const glm::mat4&) const;
	void DrawBoxHandles(const ColliderComponent&, const glm::mat4&) const;
	void DrawCircleHandles(const ColliderComponent&, const glm::mat4&) const;
	void DrawLineHandles(const ColliderComponent&, const glm::mat4&) const;
	void DrawHandle(const glm::vec2& center, const glm::mat4& projection) const;
private:
	GLuint VAO, VBO;
	Shader* shader;
	float handleSize = 12.0f;
};