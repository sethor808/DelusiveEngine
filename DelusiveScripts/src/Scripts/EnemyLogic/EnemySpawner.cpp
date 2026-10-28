#include <Scripts/EnemyLogic/EnemySpawner.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Components/ScriptComponent.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <algorithm>
#include <cmath>
#include <random>

EnemySpawner::EnemySpawner() {
	RegisterProperties();
}

void EnemySpawner::RegisterProperties() {
	//BehaviourScript's constructor already registered the base fields
	registry->Register("agent", &agent);
	registry->Register("count", &count);
	registry->Register("maxAlive", &maxAlive);
	registry->Register("interval", &interval);
	registry->Register("spawnRadius", &spawnRadius);

	//Base fields a spawner doesn't use
	registry->Hide("target");
	registry->Hide("movementSpeed");
}

void EnemySpawner::Update(float deltaTime) {
	BehaviourScript::Update(deltaTime);

	Scene* scene = owner ? owner->GetScene() : nullptr;
	if (!scene || agent.empty()) return;

	//Forget enemies that died
	std::erase_if(alive, [scene](const UUID& id) {
		Agent* enemy = scene->FindAgentByUUID(id);
		return !enemy || enemy->IsDestroyed();
	});

	if (count > 0 && spawned >= count) return;
	if (static_cast<int>(alive.size()) >= maxAlive) return;

	timer -= deltaTime;
	if (timer > 0.0f) return;

	timer = interval;
	Spawn(*scene);
}

void EnemySpawner::Spawn(Scene& scene) {
	const glm::vec2 position = owner->GetTransform().position + RandomOffset();
	const UUID id = scene.SpawnAgent(std::string(AGENT_PATH) + agent + AGENT_EXT, position);
	if (!id.IsValid()) return;

	++spawned;
	alive.push_back(id);

	//Targets saved in the agent file can't point at this scene's player, so aim them here
	Agent* enemy = scene.FindAgentByUUID(id);
	Agent* player = scene.FetchPlayerRaw();
	if (!enemy || !player) return;

	for (ScriptComponent* script : enemy->GetComponentsOfType<ScriptComponent>()) {
		script->SetTarget(player);
	}
}

glm::vec2 EnemySpawner::RandomOffset() const {
	static std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> angle(0.0f, 6.2831853f);
	std::uniform_real_distribution<float> distance(0.0f, std::max(spawnRadius, 0.0f));

	const float a = angle(rng);
	return glm::vec2(std::cos(a), std::sin(a)) * distance(rng);
}
