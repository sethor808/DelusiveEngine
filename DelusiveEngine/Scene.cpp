#include "Scene.h"
#include "GameManager.h"
#include "DelusiveAgents.h"

//TODO: If there is no camera, handle properly
Scene::Scene(DelusiveRenderer& _renderer)
	: renderer(_renderer), name("New Scene"), camera(nullptr)
{

}

Scene::~Scene() {

}

std::unique_ptr<Scene> Scene::Clone() const{
	auto cloned = std::make_unique<Scene>(renderer);
	cloned->name = this->name;

	if(gameManager) {
		cloned->gameManager = gameManager;
	}

	for (const auto& agent : agents) {
		cloned->AddAgent(agent->Clone());
	}

	for (const auto& sys : systems) {
		cloned->systems.push_back(std::unique_ptr<SceneSystem>(sys->Clone()));
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
			container.AddAgent(agent->Clone());
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
	if (gameManager) {
		return gameManager->GetScriptManager();
	}
}

void Scene::AddAgent(std::unique_ptr<Agent> _agent) {
	_agent->SetID(nextAgentID);
	_agent->SetScene(this);
	agents.push_back(std::move(_agent));
	nextAgentID++;
}

std::vector<std::unique_ptr<Agent>>& Scene::GetAgents() {
	return agents;
}

void Scene::ClearAgents() {
	agents.clear();
}

void Scene::AddSystem(std::unique_ptr<SceneSystem> sys) {
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
		system->Draw(renderer.GetUIProjection());
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
	name = "New Scene";
}

bool Scene::SaveToFile(const std::string& path) const {
	std::ofstream out(path);
	if (!out.is_open()) return false;

	out << "[Scene]" << "\n";
	out << "name=" << name << "\n";

	// Save agents
	out << "agents=" << agents.size() << "\n";
	for (const auto& agent : agents) {
		agent->SaveToFile(out);
	}

	out << "systems=" << systems.size() << "\n";
	for (auto& sys : systems) {
		sys->Serialize(out);
	}

	out << "[/Scene]" << "\n";

	return true;
}

bool Scene::LoadFromFile(const std::string& path) {
	std::ifstream in(path);
	if (!in.is_open()) return false;

	Clear(); // reset

	std::string line;
	while (std::getline(in, line)) {
		if (line.empty()) continue;

		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "name") {
			std::string rest;
			std::getline(iss, rest);
			if (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
			name = rest;
		}
		else if (token == "agents") {
			// We don’t actually need the number here, we just read [Agent] blocks
			continue;
		}
		else if (token == "[Agent") {
			std::string type;
			iss >> type; // e.g. CameraAgent]

			// trim trailing ']'
			if (!type.empty() && type.back() == ']') {
				type.pop_back();
			}

			std::unique_ptr<Agent> agent;
			if (type == "PlayerAgent") agent = std::make_unique<PlayerAgent>("");
			else if (type == "CameraAgent") agent = std::make_unique<CameraAgent>("");
			else if (type == "EnemyAgent") agent = std::make_unique<EnemyAgent>("");
			else if (type == "EnvironmentAgent") agent = std::make_unique<EnvironmentAgent>("");
			else agent = std::make_unique<PlayerAgent>(""); // fallback

			// Let the agent load its block (until [/Agent])
			agent->LoadFromFile(in);
			//agents.push_back(std::move(agent));
			AddAgent(std::move(agent));
		}
		else if (token == "systems"){
			continue;
		}
		else if (token == "[System") {
			std::string type;
			iss >> type;
			
			if (!type.empty() && type.back() == ']') {
				type.pop_back();
			}

			std::unique_ptr<SceneSystem> sys = nullptr;
			if (type == "PathfindingSystem") sys = std::make_unique<PathfindingSystem>(renderer);
			else if (type == "UIManager") sys = std::make_unique<UIManager>(renderer);

			sys->Deserialize(in);
			systems.push_back(std::move(sys));
		}
	}

	return true;
}

CameraAgent* Scene::GetMainCamera() const {
	for (auto& agent : agents) {
		if (auto cam = dynamic_cast<CameraAgent*>(agent.get())) {
			return cam;
		}
	}
	return nullptr;
}