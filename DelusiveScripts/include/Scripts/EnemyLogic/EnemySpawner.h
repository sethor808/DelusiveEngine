#pragma once
#include <Delusive/Scripting/BehaviourScript.h>
#include <Delusive/Runtime/Core/UUID.h>
#include <string>
#include <vector>

class Scene;

//Put on an invisible agent: spawns copies of an agent file around it while the game plays
class EnemySpawner : public BehaviourScript {
public:
	EnemySpawner();
	void RegisterProperties() override;

	std::string GetType() const override { return "EnemySpawner"; }

	void Update(float deltaTime) override;

private:
	//Agent file name in the agents folder, without the extension
	std::string agent;
	//0 = keep spawning forever
	int count = 5;
	int maxAlive = 3;
	float interval = 2.0f;
	float spawnRadius = 1.0f;

	float timer = 0.0f;
	int spawned = 0;
	std::vector<UUID> alive;

	void Spawn(Scene&);
	glm::vec2 RandomOffset() const;
};
