#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>

class Renderer {
public:
	static void Init();
	static void Clear();
	static void Shutdown();
	static void OnResize(int width, int height);
	static void GetWindowSize(int&, int&);
	static const glm::mat4& GetProjection();
	static GLuint CreateFallbackWhiteTexture();

	//Camera stuff
};