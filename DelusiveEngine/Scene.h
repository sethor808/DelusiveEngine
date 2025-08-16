#pragma once
#include <iostream>
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Sprite.h"
#include "Agent.h"
#include "DelusiveAgents.h"
#include "DelusiveUtils.h"
#include "SceneSystem.h"
#include "PhysicsSystem.h"

class Scene {
public:
	Scene();
	Scene(std::string);
	~Scene();

	Scene(Scene&&) noexcept = default;
	Scene& operator=(Scene&&) noexcept = default;
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	std::unique_ptr<Scene> Clone() const;

	//Agent managmenet
	void AddAgent(std::unique_ptr<Agent>);
	std::vector<std::unique_ptr<Agent>>& GetAgents();
	void ClearAgents();

	//System management
	void AddSystem(std::unique_ptr<SceneSystem>);
	template<typename T> T* GetSystem();
	std::vector<std::unique_ptr<SceneSystem>>& GetSystems();

	//Camera stuff
	CameraAgent* GetMainCamera() const;

	void Update(float deltaTime);
	void Draw(const ColliderRenderer& renderer, const glm::mat4& projection) const;
	void HandleInput(const PlayerInputState& input);
	void HandleMouse(const glm::vec2&, bool);
	void CloneInto(Scene&) const;
	void Clear();
	std::string GetName() { return name; }
	void SetName(const std::string& _name) { name = _name; }

	bool SaveToFile(const std::string& path) const;
	bool LoadFromFile(const std::string& path);

private:
	std::string name;
	CameraAgent* camera;
	static PhysicsSystem physicsSystem;
	std::vector<std::unique_ptr<Agent>> agents;
	std::vector<std::unique_ptr<SceneSystem>> systems;
};