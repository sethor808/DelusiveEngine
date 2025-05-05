#pragma once
#include <glm/glm.hpp>

class Renderer {
public:
	static void Init();
	static void Clear();
	static void Shutdown();
	static void OnResize(int width, int height);
	static const glm::mat4& GetProjection();
};