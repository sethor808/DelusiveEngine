#pragma once
#include <string>
#include <GL/glew.h>
#include "Texture.h"
#include "Shader.h"

struct DelusiveTexture {
	std::string texturePath = "";
	std::string previousTexturePath = "";
	GLuint VAO = 0, VBO = 0;
	Texture* texture = nullptr;
	Shader* shader = nullptr;

	DelusiveTexture() = default;

	~DelusiveTexture() {
		if (VAO) glDeleteVertexArrays(1, &VAO);
		if (VBO) glDeleteBuffers(1, &VBO);
		if (texture) delete texture;
		if (shader) delete shader;
	}

	void Init(
		const std::string& shaderVert = "shaders/vertex.glsl",
		const std::string& shaderFrag = "shaders/fragment.glsl")
	{
		shader = new Shader(shaderVert.c_str(), shaderFrag.c_str());
		if (!texturePath.empty()) {
			texture = new Texture(texturePath.c_str());
		}

		float vertices[] = {
			// pos       // tex
			-0.5f, -0.5f,  0.0f, 0.0f,
			 0.5f, -0.5f,  1.0f, 0.0f,
			 0.5f,  0.5f,  1.0f, 1.0f,

			 0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.0f, 0.0f
		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}

	void SetTexture(const std::string& path) {
		texturePath = path;
		if (texture) { delete texture; texture = nullptr; }
		texture = new Texture(path.c_str());
	}

	void Draw(const glm::mat4& model,
		const glm::mat4& view,
		const glm::mat4& projection) const
	{
		if (!shader || !texture) return;

		shader->Use();

		glActiveTexture(GL_TEXTURE0);
		texture->Bind();

		GLint loc = glGetUniformLocation(shader->GetID(), "tex");
		if (loc >= 0) glUniform1i(loc, 0);

		shader->SetMat4("model", glm::value_ptr(model));
		shader->SetMat4("view", glm::value_ptr(view));
		shader->SetMat4("projection", glm::value_ptr(projection));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
};

struct DelusiveFont {
	int fontSize = 16;
	std::string fontPath = "";
	std::string previousFontPath = "";
};

struct DelusiveScript {
	std::string scriptName;
};