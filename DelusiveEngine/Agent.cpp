#include "Agent.h"
#include "Component.h"
#include "DelusiveComponents.h"
#include "Renderer.h"
#include <limits>
#include <sstream>

Agent::Agent() {

}

Agent::~Agent() {

}

void Agent::HandleMouse(const glm::vec2& worldMouse, bool mouseDown) {
	for (auto& c : components) {
		c->HandleMouse(worldMouse, mouseDown);
	}
}

void Agent::SetPosition(const glm::vec2& pos) {
	transform.position = pos;
}

void Agent::SetRotation(const float rotation) {
	transform.rotation = rotation;
}

void Agent::SetScale(const glm::vec2 scale) {
	transform.scale = scale;
}

GLuint Agent::RenderAgentToTexture(int width, int height) {
	GLuint framebuffer, textureColorbuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// Create texture
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Attach to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	// Optional depth buffer
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "[RenderAgentToTexture] Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return 0;
	}

	// Set viewport and clear
	glViewport(0, 0, width, height);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float halfWidth = width * 0.5f;
	float halfHeight = height * 0.5f;
	constexpr float pixelsPerUnit = 64.0f;
	float halfWidthUnits = width / (2.0f * pixelsPerUnit);
	float halfHeightUnits = height / (2.0f * pixelsPerUnit);
	glm::mat4 projection = glm::ortho(
		-halfWidthUnits, halfWidthUnits,
		-halfHeightUnits, halfHeightUnits,
		-1.0f, 1.0f
	);

	this->Draw(projection);

	// Cleanup
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_BLEND);
	glDeleteRenderbuffers(1, &rbo);
	glDeleteFramebuffers(1, &framebuffer);

	return textureColorbuffer;
}

void Agent::AddRawComponent(std::unique_ptr<Component> component) {
	component->SetOwner(this);
	component->SetID(nextComponentID++);
	components.push_back(std::move(component));
}

Component* Agent::GetComponentByName(const std::string & name) {
	for (auto& comp : components) {
		if (comp->GetName() == name) {
			return comp.get();
		}
	}
	return nullptr;
}

Component* Agent::GetComponentByID(uint64_t id) {
	for (auto& comp : components) {
		if (comp->GetID() == id) {
			return comp.get();
		}
	}
	return nullptr;
}

TransformComponent& Agent::GetTransform() {
	return transform;
}

const TransformComponent& Agent::GetTransform() const {
	return transform;
}

void Agent::SaveToFile(std::ofstream& out) const {
	out << "name " << name << "\n";
	out << "transform "
		<< transform.position.x << " " << transform.position.y << " "
		<< transform.rotation << " "
		<< transform.scale.x << " " << transform.scale.y << "\n";

	out << "components " << components.size() << "\n";
	for (const auto& comp : components) {
		out << comp->GetType() << "\n";
		comp->Serialize(out);
	}
}

void Agent::SaveToFile(const std::string& filePath) const {
	std::ofstream out(filePath);
	if (!out.is_open()) return;
	SaveToFile(out);
}

void Agent::LoadFromFile(std::ifstream& in) {
	if (!in) return;

	std::string line;
	components.clear();

	while (std::getline(in, line)) {
		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "name") {
			std::getline(iss, name);
			if (!name.empty() && name[0] == ' ') name.erase(0, 1);  // trim space
		}
		else if (token == "transform") {
			iss >> transform.position.x >> transform.position.y;
			iss >> transform.rotation;
			iss >> transform.scale.x >> transform.scale.y;
		}
		else if (token == "components") {
			size_t count = 0;
			iss >> count;
			for (size_t i = 0; i < count; ++i) {
				std::string typeLine;
				if (!std::getline(in, typeLine)) break;

				std::istringstream typeStream(typeLine);
				std::string type;
				typeStream >> type;

				Component* comp = nullptr;
				if (type == "SpriteComponent") comp = new SpriteComponent("");
				else if (type == "SolidCollider") comp = new SolidCollider();
				else if (type == "TriggerCollider") comp = new TriggerCollider();
				else if (type == "HurtboxCollider") comp = new HurtboxCollider();
				else if (type == "HitboxCollider") comp = new HitboxCollider();
				else if (type == "StatsComponent") comp = new StatsComponent();

				if (comp) {
					comp->SetOwner(this);
					comp->Deserialize(in);
					components.push_back(std::unique_ptr<Component>(comp));
				}
			}
		}
	}
}

void Agent::LoadFromFile(const std::string& filePath) {
	std::ifstream in(filePath);
	if (!in.is_open()) return;
	LoadFromFile(in);
}

void Agent::RemoveComponentByPointer(Component* target) {
	components.erase(
		std::remove_if(
			components.begin(),
			components.end(),
			[target](const std::unique_ptr<Component>& c) {
				return c.get() == target;
			}
		),
		components.end()
	);
}

const std::vector<std::unique_ptr<Component>>& Agent::GetComponents() const {
	return components;
}