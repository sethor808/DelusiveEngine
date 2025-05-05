#pragma once
#include <iostream>
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Sprite.h"
#include "Agent.h"
#include "PlayerAgent.h"

class Scene {
public:
	Scene(std::string);
	~Scene();

	Scene(Scene&&) noexcept = default;
	Scene& operator=(Scene&&) noexcept = default;

	// optionally delete copy to enforce move-only
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	void AddSprite(Sprite* sprite);
	void RemoveSprite(Sprite* sprite);

	void AddAgent(std::unique_ptr<Agent>);
	std::vector<std::unique_ptr<Agent>>& GetAgents();

	void Update(float deltaTime);
	void Draw(const ColliderRenderer& renderer, const glm::mat4& projection) const;

	bool SaveToFile(const std::string& path) const;
	bool LoadFromFile(const std::string& path);

private:
	std::string name;
	std::vector<Sprite*> sprites;
	std::vector<std::unique_ptr<Agent>> agents;
};