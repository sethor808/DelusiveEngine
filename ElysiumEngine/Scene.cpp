#include "Scene.h"

Scene::Scene(std::string _name) {
	name = _name;
}

Scene::~Scene() {

}

void Scene::AddSprite(Sprite* sprite) {
	if (sprite) {
		sprites.push_back(sprite);
	}
}

void Scene::RemoveSprite(Sprite* sprite) {
	auto it = std::find(sprites.begin(), sprites.end(), sprite);
	if (it != sprites.end()) {
		sprites.erase(it);
	}
}

void Scene::AddAgent(std::unique_ptr<Agent> _agent) {
	agents.push_back(std::move(_agent));
}

std::vector<std::unique_ptr<Agent>>& Scene::GetAgents() {
	return agents;
}

void Scene::Update(float deltaTime) {
	for (auto& agent : agents) {
		agent->Update(deltaTime);
	}
}

void Scene::Draw(const ColliderRenderer& renderer, const glm::mat4& projection) const {
	for (auto& agent : agents) {
		agent->Draw(renderer, projection);
	}
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
				auto agent = std::make_unique<PlayerAgent>("assets/sprites/star.png", ""); //TODO: Replace with proper pathing later
				agent->LoadFromFile(in);
				agents.push_back(std::move(agent));
			}
		}
	}

	return true;
}