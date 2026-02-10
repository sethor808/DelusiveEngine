#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <memory>
#include <vector>
#include <stack>
#include <Delusive/Internal/Rendering/Shader.h>
#include <Delusive/Internal/Rendering/Font.h>

struct RenderCommand;
class TextureManager;
class Agent;

class DelusiveRenderer {
public:
	DelusiveRenderer();
	~DelusiveRenderer();

	void Init();
	void InitTextRenderer();
	void Clear();
	void Shutdown();

	void SetViewProjection(glm::mat4& view, glm::mat4& projection) { this->view = view; this->projection = projection; }

	void GenerateProjection();
	void OnResize(int width, int height);
	void GetWindowSize(int&, int&);
	const glm::mat4& GetProjection() const;

	//Draw functions
	GLuint GetTexture(const std::string&);
	Shader* GetDefaultShader();
	void Submit(const RenderCommand&);
	void Flush();

	GLuint CreateFallbackWhiteTexture();
	glm::mat4 GetUIProjection();
	void BeginUIRenderPass();
	void EndUIRenderPass();

	Shader* GetDefaultUIShader();

	//Drawing tools
	void PushAlpha(float);
	void PopAlpha();
	void SetAlpha(float alpha);
	float GetAlpha() const { return currentAlpha; }

	void DebugDrawLine(glm::vec2, glm::vec2, glm::vec4);
	void DebugDrawRect(glm::vec2, float, glm::vec4);
	void DrawText(const std::string&, glm::vec2, float, glm::vec4, const glm::mat4&);
	void DrawRect(
		const glm::vec2& pos,            // Bottom-left or top-left corner
		const glm::vec2& size,           // Width, height
		const glm::vec4& color,          // RGBA color
		const glm::mat4& projection,     // UI projection matrix
		Shader* shader = nullptr,        // Optional shader (uses default if null)
		GLuint texture = 0               // Optional texture (0 = no texture)
	);

	//Camera stuff
	float GetPixelScale();

    //Thumbnail generation
    GLuint RenderAgentToTexture(Agent&, int, int);
private:
	std::unique_ptr<TextureManager> textureManager;
	std::vector<RenderCommand> commands;

	glm::mat4 projection;
	glm::mat4 view;
	int width = 800;
	int height = 600;

	float currentAlpha = 1.0f;
	std::stack<float> alphaStack;

	//Text stuff
	GLuint textVAO = 0;
	GLuint textVBO = 0;

	//Rect stuff
	GLuint quadVAO = 0;
	GLuint quadVBO = 0;

	std::unique_ptr<Shader> defaultShader;
	std::unique_ptr<Shader> textShader; // Make sure Shader is included
	std::unique_ptr<Shader> uiShader;
	std::unique_ptr<Font> defaultFont;
};