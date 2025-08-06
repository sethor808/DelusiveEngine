#include "Scene.h"
//TODO: If there is no camera, handle properly
Scene::Scene()
	: name("New Scene"), camera(nullptr)
{

}

Scene::Scene(std::string _name)
	: name(std::move(_name)), camera(nullptr)
{
	
}

Scene::~Scene() {

}

std::unique_ptr<Scene> Scene::Clone() const{
	auto cloned = std::make_unique<Scene>(name);

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

void Scene::AddAgent(std::unique_ptr<Agent> _agent) {
	agents.push_back(std::move(_agent));
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
}

void Scene::Draw(const ColliderRenderer& renderer, const glm::mat4& projection) const {
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
				collider->Draw(projection);
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

	out << "name " << name << "\n";
	out << "agents " << agents.size() << "\n";
	for (const auto& agent : agents) {
		agent->SaveToFile(out);
	}
	
	return true;
}

bool Scene::LoadFromFile(const std::string& path) {
	std::ifstream in(path);
	if (!in.is_open()) return false;

	agents.clear();

	std::string token;
	while (in >> token) {
		if (token == "name") {
			size_t count;
			in >> count;
			for (size_t i = 0; i < count; ++i) {
				auto agent = std::make_unique<PlayerAgent>(""); //TODO: Replace with proper pathing later
				agent->LoadFromFile(in);
				agents.push_back(std::move(agent));
			}
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