#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Shader.h"

class Renderer {
public:
	static void Init();
	static void InitTextRenderer();
	static void Clear();
	static void Shutdown();
	static void GenerateProjection();
	static void OnResize(int width, int height);
	static void GetWindowSize(int&, int&);
	static const glm::mat4& GetProjection();
	static GLuint CreateFallbackWhiteTexture();
	static glm::mat4 GetUIProjection();
	static void BeginUIRenderPass();
	static void EndUIRenderPass();

	//Drawing tools
	static void DebugDrawLine(glm::vec2, glm::vec2, glm::vec4);
	static void DebugDrawRect(glm::vec2, float, glm::vec4);
	static void DrawText(const std::string&, glm::vec2, float, glm::vec4, const glm::mat4&);
	static void DrawRect(
		const glm::vec2& pos,            // Bottom-left or top-left corner
		const glm::vec2& size,           // Width, height
		const glm::vec4& color,          // RGBA color
		const glm::mat4& projection,     // UI projection matrix
		Shader* shader = nullptr,        // Optional shader (uses default if null)
		GLuint texture = 0               // Optional texture (0 = no texture)
	);
	//Camera stuff
};