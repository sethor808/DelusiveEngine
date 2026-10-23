#pragma once
#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

class Texture;

class TextureManager {
public:
	GLuint Load(const std::string&);
	//Pixel size of the image, loading it if needed; 0,0 if it cannot be loaded
	glm::ivec2 GetSize(const std::string&);
	void Bind(const std::string&, GLuint unit);
	void UnloadAll();
private:
	std::unordered_map<std::string, std::unique_ptr<Texture>> cache;
	GLuint LoadFromFile(const std::string& path);
};