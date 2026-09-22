#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveCoreIncludes.h>
#include <Delusive/Runtime/Core/GameManager.h>
#include <Delusive/Runtime/Agents/DelusiveAgents.h>
#include <Delusive/Runtime/Core/DelusiveData.h>

//TODO: If there is no camera, handle properly
Scene::Scene(DelusiveInstance& instance)
	: instance(instance), name("New Scene"), camera(nullptr)
{
    //inventoryLink = gameManager->GetInventoryLink();
}

Scene::~Scene() {

}

void Scene::LinkGameManager(GameManager* gm) {
    gameManager = gm;
    inventoryLink = gameManager->GetInventoryLink();
}

std::unique_ptr<Scene> Scene::Clone() {
	auto cloned = std::make_unique<Scene>(instance);
	cloned->name = this->name;

	if(gameManager) {
		cloned->gameManager = gameManager;
	}

	for (const auto& agent : agents) {
		cloned->AddAgent(agent->Clone(cloned.get()));
	}

	for (const auto& sys : systems) {
		cloned->AddSystem(sys->Clone());
	}

	return cloned;
}

void Scene::CloneInto(Scene& container) const {
	container.name = this->name;
	container.Clear(); // Clean existing contents before cloning

	if (gameManager) {
		container.gameManager = gameManager;
	}

	// Clone Agents
	for (const auto& agent : agents) {
		if (agent) {
			container.AddAgent(agent->Clone(&container));
		}
	}

    // After all agents are added
    for (auto& agent : container.agents) {
        for (auto& comp : agent->GetComponents()) {
            if (auto* scriptComp = dynamic_cast<ScriptComponent*>(comp.get())) {
                if (auto* script = scriptComp->GetScript()) {
                    if (script) {
                        script->RelocateReferences();
                    }
                }
            }
        }
    }

	// Clone Systems
	for (const auto& system : systems) {
		if (system) {
			container.AddSystem(system->Clone());
		}
	}

	// Update camera pointer
	container.camera = nullptr;
	for (auto& agent : container.agents) {
		if (auto* cam = dynamic_cast<CameraAgent*>(agent.get())) {
			container.camera = cam;
			break;
		}
	}
}

bool Scene::HasCamera() const {
	for (const auto& agent : agents) {
		if (dynamic_cast<CameraAgent*>(agent.get())) {
			return true;
		}
	}
	return false;
}

ScriptManager& Scene::GetScriptManager() const {
	if (gameManager != nullptr) {
		return gameManager->GetScriptManager();
	}
	throw std::runtime_error("Scene::GetScriptManager() - GameManager is null");
}

void Scene::AddAgent(std::unique_ptr<Agent> _agent) {
    if (!_agent->GetID().IsValid()) {
        _agent->SetID(UUID::GenerateRandom());
    }
    _agent->LinkScene(this);
    agentLookup[_agent->GetID()] = _agent.get();
    agents.push_back(std::move(_agent));
}
    

Agent* Scene::FindAgentByUUID(UUID targetID) {
    auto it = agentLookup.find(targetID);
    if (it != agentLookup.end()) {
        return it->second;
    }
    return nullptr;
}

PlayerAgent* Scene::FetchPlayer() {
	for (const auto& agent : agents) {
		if (agent) {
			if (auto player = dynamic_cast<PlayerAgent*>(agent.get())) {
				return player;
			}
		}
	}
	return nullptr;
}

Agent* Scene::FetchPlayerRaw() {
	for (const auto& agent : agents) {
		if (agent) {
			if (auto player = dynamic_cast<PlayerAgent*>(agent.get())) {
				return player;
			}
		}
	}
	return nullptr;
}

std::vector<std::unique_ptr<Agent>>& Scene::GetAgents() {
	return agents;
}

void Scene::ClearAgents() {
	agents.clear();
    agentLookup.clear();
}

void Scene::AddSystem(std::unique_ptr<SceneSystem> sys) {
	sys->LinkScene(this);
    sys->Init();
	systems.push_back(std::move(sys));
}

std::vector<std::unique_ptr<SceneSystem>>& Scene::GetSystems() {
	return systems;
}

template<typename T>
T* Scene::GetSystem() {
	for (auto& sys : systems) {
		if (auto ptr = dynamic_cast<T*>(sys.get())) {
			return ptr;
		}
	}
	return nullptr;
}

void Scene::Update(float deltaTime) {
	if (!camera) {
		for (auto& agent : agents) {
			// Use dynamic_cast to check if agent is a CameraAgent
			if (auto camAgent = dynamic_cast<CameraAgent*>(agent.get())) {
				camera = camAgent;
				break;  // found the first CameraAgent, stop looping
			}
		}
		if (!camera) return;
	}

	if (camera) camera->Update(deltaTime);

	for (auto& sys : systems) {
		sys->Update(deltaTime);
	}

	for (auto& agent : agents) {
		agent->Update(deltaTime);
	}

	physicsSystem.HandleCollisions(agents);
}

void Scene::Draw(const ColliderRenderer& colRenderer, const glm::mat4& projection) const {
	struct RenderEntry {
		SpriteComponent* sprite;
		float sortY;
		bool isForeground;
	};

	std::vector<RenderEntry> renderQueue;
	renderQueue.reserve(agents.size() * 2); // Conservative estimate, avoids reallocations

	for (const auto& agent : agents) {
		const glm::vec2 agentPos = agent->GetTransform().position;

		// Collect enabled sprites
		for (SpriteComponent* sprite : agent->GetComponentsOfType<SpriteComponent>()) {
			if (sprite->IsEnabled()) {
				renderQueue.push_back({ sprite, agentPos.y, sprite->isForeground });
			}
		}

		// Immediately draw enabled colliders (no sorting needed)
		for (const ColliderComponent* collider : agent->GetComponentsOfType<ColliderComponent>()) {
			if (collider->IsEnabled()) {
				collider->Draw(colRenderer, projection);
			}
		}
	}

	// Sort sprite draw order (foreground sprites on top, then lower Y = top)
	std::sort(renderQueue.begin(), renderQueue.end(), [](const RenderEntry& a, const RenderEntry& b) {
		if (a.isForeground != b.isForeground)
			return !a.isForeground && b.isForeground;
		return a.sortY < b.sortY;
		});

	// Draw sorted sprites
	for (const RenderEntry& entry : renderQueue) {
		entry.sprite->Draw(projection);
	}

	//Renderer::BeginUIRenderPass();
	for (auto& system : systems) {
		system->Draw(instance.renderer.GetUIProjection());
	}
	//Renderer::EndUIRenderPass();
}

void Scene::HandleInput(const PlayerInputState& input) {
	for (auto& agent : agents) {
		agent->HandleInput(input);
	}
}

void Scene::HandleMouse(const glm::vec2& worldMouse, bool mouseDown) {
	for (auto& agent : agents) {
		agent->HandleMouse(worldMouse, mouseDown);
	}
}

void Scene::Clear() {
	agents.clear();
	systems.clear();
    agentLookup.clear();
	name = "New Scene";
}

CameraAgent* Scene::GetMainCamera() const {
	for (auto& agent : agents) {
		if (auto cam = dynamic_cast<CameraAgent*>(agent.get())) {
			return cam;
		}
	}
	return nullptr;
}
#pragma region File IO

namespace {
	std::unique_ptr<Agent> CreateAgentByType(const std::string& type, DelusiveInstance& instance) {
		if (type == "PlayerAgent")      return std::make_unique<PlayerAgent>(instance);
		if (type == "CameraAgent")      return std::make_unique<CameraAgent>(instance);
		if (type == "EnemyAgent")       return std::make_unique<EnemyAgent>(instance);
		if (type == "EnvironmentAgent") return std::make_unique<EnvironmentAgent>(instance);
		return nullptr;
	}

	std::unique_ptr<SceneSystem> CreateSystemByType(const std::string& type, DelusiveInstance& instance) {
		if (type == "UIManager")         return std::make_unique<UIManager>(instance);
		if (type == "PathfindingSystem") return std::make_unique<PathfindingSystem>(instance);
		return nullptr;
	}

	std::string JoinIDs(const std::vector<UUID>& ids) {
		std::ostringstream out;
		for (const UUID& id : ids) out << id.ToString() << " ";
		return out.str();
	}

	std::vector<UUID> SplitIDs(const std::string& text) {
		std::vector<UUID> ids;
		std::istringstream in(text);
		std::string token;

		while (in >> token) {
			UUID id;
			id.FromString(token);
			if (id.IsValid()) ids.push_back(id);
		}

		return ids;
	}
}

bool Scene::SaveToFile(const std::string& path) const {
	std::vector<DelusiveParser::DataBlock> blocks;

	//Scene block carries the name plus what it owns
	DelusiveParser::DataBlock sceneBlock;
	sceneBlock.category = "Scene";
	sceneBlock.id = id.IsValid() ? id : UUID::GenerateRandom();
	sceneBlock.properties["name"] = name;

	std::vector<UUID> agentIDs;
	std::vector<UUID> systemIDs;

	for (const auto& agent : agents) {
		if (agent) agentIDs.push_back(agent->GetID());
	}
	for (const auto& sys : systems) {
		if (sys) systemIDs.push_back(sys->GetID());
	}

	sceneBlock.properties["agents"] = JoinIDs(agentIDs);
	sceneBlock.properties["systems"] = JoinIDs(systemIDs);
	blocks.push_back(std::move(sceneBlock));

	//Agents emit themselves and their components, flat
	for (const auto& agent : agents) {
		if (agent) agent->CollectBlocks(blocks);
	}

	for (const auto& sys : systems) {
		if (!sys) continue;

		DelusiveParser::DataBlock block;
		sys->Serialize(block);
		block.id = sys->GetID();
		blocks.push_back(std::move(block));
	}

	std::ofstream out(path);
	if (!out) {
		std::cerr << "[Scene] Cannot write " << path << std::endl;
		return false;
	}

	out << std::setprecision(std::numeric_limits<float>::max_digits10);

	for (const DelusiveParser::DataBlock& block : blocks) {
		DelusiveParser::WriteBlock(out, block);
		out << "\n";
	}

	return static_cast<bool>(out);
}

bool Scene::LoadFromFile(const std::string& path) {
	std::ifstream in(path);
	if (!in) {
		std::cerr << "[Scene] Cannot open " << path << std::endl;
		return false;
	}

	//Every block in this file becomes a recipe the agents can pull from
	std::vector<DelusiveParser::DataBlock> blocks = DelusiveParser::ParseFile(in);

	for (auto& block : blocks) {
		if (block.id.IsValid()) {
			instance.delusiveLibrary.Add(block, path);
		}
	}

	const DelusiveParser::DataBlock* sceneBlock = nullptr;
	for (const auto& block : blocks) {
		if (block.category == "Scene") {
			sceneBlock = &block;
			break;
		}
	}

	if (!sceneBlock) {
		std::cerr << "[Scene] No Scene block in " << path << std::endl;
		return false;
	}

	Clear();

	id = sceneBlock->id;

	auto nameProp = sceneBlock->properties.find("name");
	if (nameProp != sceneBlock->properties.end()) name = nameProp->second;

	//Systems first so agents can find them during resolve
	auto systemList = sceneBlock->properties.find("systems");
	if (systemList != sceneBlock->properties.end()) {
		for (const UUID& sysID : SplitIDs(systemList->second)) {
			const DelusiveParser::DataBlock* recipe = instance.delusiveLibrary.Find(sysID);
			if (!recipe) continue;

			std::unique_ptr<SceneSystem> sys = CreateSystemByType(recipe->type, instance);
			if (!sys) {
				std::cerr << "[Scene] Unknown system type: " << recipe->type << std::endl;
				continue;
			}

			sys->Deserialize(const_cast<DelusiveParser::DataBlock&>(*recipe));
			sys->SetID(sysID);
			AddSystem(std::move(sys));
		}
	}

	auto agentList = sceneBlock->properties.find("agents");
	if (agentList != sceneBlock->properties.end()) {
		for (const UUID& agentID : SplitIDs(agentList->second)) {
			const DelusiveParser::DataBlock* recipe = instance.delusiveLibrary.Find(agentID);
			if (!recipe) {
				std::cerr << "[Scene] Missing agent recipe " << agentID.ToString() << std::endl;
				continue;
			}

			std::unique_ptr<Agent> agent = CreateAgentByType(recipe->type, instance);
			if (!agent) {
				std::cerr << "[Scene] Unknown agent type: " << recipe->type << std::endl;
				continue;
			}

			agent->LinkScene(this);
			agent->Deserialize(const_cast<DelusiveParser::DataBlock&>(*recipe));
			agent->SetID(agentID);
			AddAgent(std::move(agent));
		}
	}

	return true;
}

#pragma endregion
