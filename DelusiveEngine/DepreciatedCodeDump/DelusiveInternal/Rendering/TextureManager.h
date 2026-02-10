#pragma once
#include <GL/glew.h>
#include <string>
#include <unordered_map>
#include <memory>

class Texture;

class TextureManager {
public:
	GLuint Load(const std::string&);
	void Bind(const std::string&, GLuint unit);
	void UnloadAll();
private:
	std::unordered_map<std::string, std::unique_ptr<Texture>> cache;
	GLuint LoadFromFile(const std::string& path);
};