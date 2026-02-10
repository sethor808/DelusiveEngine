#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera2D {
public:
	glm::vec2 position = { 0.0f, 0.0f };

	glm::mat4 GetViewMatrix() const {
		return glm::translate(glm::mat4(1.0f), glm::vec3(-position, 0.0f));
	}

	void Move(float dx, float dy) {
		position.x = dx;
		position.y = dy;
	}
};
