#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveClone.h>
#include <Delusive/Runtime/Core/DelusiveCoreIncludes.h>
#include <Delusive/Runtime/Core/GameManager.h>
#include <Delusive/Runtime/Agents/DelusiveAgents.h>
#include <Delusive/Runtime/Core/DelusiveData.h>
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <algorithm>
#include <iomanip>
#include <sstream>

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

bool Scene::RemoveAgent(const UUID& agentID) {
	auto found = std::find_if(agents.begin(), agents.end(),
		[&](const std::unique_ptr<Agent>& a) { return a && a->GetID() == agentID; });
	if (found == agents.end()) return false;

	if (camera == found->get()) camera = nullptr;
	agentLookup.erase(agentID);
	agents.erase(found);
	return true;
}

void Scene::ClearAgents() {
	agents.clear();
    agentLookup.clear();
	camera = nullptr;
}

void Scene::AddSystem(std::unique_ptr<SceneSystem> sys) {
	if (!sys->GetID().IsValid()) sys->SetID(UUID::GenerateRandom());
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

	physics.Step(agents);
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
	camera = nullptr;
	name = "New Scene";
	//A cleared scene is a new scene - it must not save over the old one's ids
	id = UUID();
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

void Scene::CollectBlocks(std::vector<DelusiveParser::DataBlock>& out) const {
	//Scene block carries the name plus what it owns
	DelusiveParser::DataBlock sceneBlock;
	sceneBlock.category = "Scene";
	sceneBlock.id = id;
	{
		std::ostringstream quoted;
		quoted << std::quoted(name);
		sceneBlock.properties["name"] = quoted.str();
	}

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
	out.push_back(std::move(sceneBlock));

	//Agents emit themselves and their components, flat
	for (const auto& agent : agents) {
		if (agent) agent->CollectBlocks(out);
	}
	for (const auto& sys : systems) {
		if (sys) sys->CollectBlocks(out);
	}
}

bool Scene::SaveToFile(const std::string& path, bool* reloaded) {
	if (reloaded) *reloaded = false;
	if (!id.IsValid()) id = UUID::GenerateRandom();

	std::vector<DelusiveParser::DataBlock> blocks;
	CollectBlocks(blocks);

	//Also replaces the library's copy of this file, so a reload sees what was just saved
	DelusiveLibrary::IDRemap remap;
	if (!instance.delusiveLibrary.WriteFile(path, std::move(blocks), &remap)) return false;

	//Saved as a copy - reload so the live scene carries the copy's fresh ids
	if (!remap.empty()) {
		if (reloaded) *reloaded = true;
		return LoadFromFile(path);
	}

	return true;
}

bool Scene::LoadFromFile(const std::string& path) {
	//Re-reads the file so edits made since startup are picked up. Every block in
	//it becomes a recipe the agents can pull from.
	if (!instance.delusiveLibrary.LoadFile(path)) return false;

	for (const DelusiveParser::DataBlock* block : instance.delusiveLibrary.ListFile(path)) {
		if (block->category == "Scene") return Build(*block);
	}

	std::cerr << "[Scene] No Scene block in " << path << std::endl;
	return false;
}

bool Scene::LoadFromBlocks(const std::vector<DelusiveParser::DataBlock>& blocks) {
	auto sceneBlock = std::find_if(blocks.begin(), blocks.end(),
		[](const DelusiveParser::DataBlock& b) { return b.category == "Scene"; });
	if (sceneBlock == blocks.end()) return false;

	//The blocks stand in for the library while the scene is rebuilt
	DelusiveLibrary::Overlay overlay(instance.delusiveLibrary, blocks);
	return Build(*sceneBlock);
}

bool Scene::Build(const DelusiveParser::DataBlock& sceneBlock) {
	Clear();

	id = sceneBlock.id;

	auto nameProp = sceneBlock.properties.find("name");
	if (nameProp != sceneBlock.properties.end()) {
		//Quoted like every other string property; older saves wrote it bare
		std::istringstream in(nameProp->second);
		if (in.peek() == '"') in >> std::quoted(name);
		else name = nameProp->second;
	}

	//Systems first so agents can find them during resolve
	auto systemList = sceneBlock.properties.find("systems");
	if (systemList != sceneBlock.properties.end()) {
		for (const UUID& sysID : SplitIDs(systemList->second)) {
			const DelusiveParser::DataBlock* recipe = instance.delusiveLibrary.Find(sysID);
			if (!recipe) continue;

			std::unique_ptr<SceneSystem> sys = DelusiveBuild<SceneSystem>(*recipe, instance);
			if (!sys) continue;

			AddSystem(std::move(sys));
		}
	}

	auto agentList = sceneBlock.properties.find("agents");
	if (agentList != sceneBlock.properties.end()) {
		for (const UUID& agentID : SplitIDs(agentList->second)) {
			const DelusiveParser::DataBlock* recipe = instance.delusiveLibrary.Find(agentID);
			if (!recipe) {
				std::cerr << "[Scene] Missing agent recipe " << agentID.ToString() << std::endl;
				continue;
			}

			std::unique_ptr<Agent> agent = Agent::FromRecipe(*recipe, instance, this);
			if (!agent) continue;

			AddAgent(std::move(agent));
		}
	}

	return true;
}

#pragma endregion
