#pragma once
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <iostream>
#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <Delusive/Runtime/Utils/DelusiveUtils.h>
#include <Delusive/Runtime/Scene/SceneSystem.h>
#include <Delusive/Runtime/Core/PhysicsSystem.h>
#include <Delusive/Runtime/Scene/DelusiveSystems.h>
#include <Delusive/Runtime/Core/UUID.h>

//Forward declarations
class Agent;
class PlayerAgent;
class CameraAgent;
class GameManager;
class ScriptManager;
class DelusiveInventory;

class Scene {
public:
	Scene() = delete;
	Scene(DelusiveInstance&);
	~Scene();

	Scene(Scene&&) noexcept = default;
	Scene& operator=(Scene&&) noexcept = default;
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	std::unique_ptr<Scene> Clone();

	bool HasCamera() const;

	//Ownership methods
    void LinkGameManager(GameManager*);
    bool HasGameManager() const { if (gameManager) return true; return false; }
	GameManager* GetGameManager() const { return gameManager; }
	ScriptManager& GetScriptManager() const;
    DelusiveInventory* GetInventoryLink() { return inventoryLink; }

	//Agent managmenet
	void AddAgent(std::unique_ptr<Agent>);
    std::vector<std::unique_ptr<Agent>>& GetAgents();
    Agent* FindAgentByUUID(UUID targetID);
	PlayerAgent* FetchPlayer();
	Agent* FetchPlayerRaw();
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

	UUID GetID() const { return id; }
	void SetID(UUID newID) { id = newID; }

	//Flat file I/O - every agent, component and system is written as its own block
	//Saving to a file other than its own gives the copy fresh ids and reloads from
	//it - reloaded tells the caller every Agent/Component pointer is now invalid
	bool SaveToFile(const std::string& path, bool* reloaded = nullptr);
	bool LoadFromFile(const std::string& path);
	//The scene as it would be saved: its own block first, then every agent and system
	void CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const;
	//Rebuilds the scene from blocks in memory - undo uses it; nothing is read from disk
	bool LoadFromBlocks(const std::vector<DelusiveParser::DataBlock>& blocks);
	//Takes the agent out of the scene and every lookup that points at it
	bool RemoveAgent(const UUID& agentID);

    template<typename T>
    bool ResolveID(DelusiveLink<T>&);
private:
	bool Build(const DelusiveParser::DataBlock& sceneBlock);

	GameManager* gameManager = nullptr;
    DelusiveInventory* inventoryLink = nullptr;
    DelusiveInstance& instance;
	UUID id;
	std::string name;
	CameraAgent* camera = nullptr;
    std::unordered_map<UUID, Agent*, UUID::Hash> agentLookup;
	//Per scene - contact state must not leak between the editor and play scenes
	PhysicsSystem physics;
	std::vector<std::unique_ptr<Agent>> agents;
	std::vector<std::unique_ptr<SceneSystem>> systems;
};