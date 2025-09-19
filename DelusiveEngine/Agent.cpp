#include "Agent.h"
#include "Component.h"
#include "Scene.h"
#include "DelusiveRegistry.h"
#include "DelusiveMacros.h"
#include "DelusiveComponents.h"
#include "DelusiveRenderer.h"
#include <limits>
#include <sstream>

Agent::Agent()
	: registry(std::make_unique<PropertyRegistry>())
{
	RegisterProperties();
}

Agent::~Agent() {

}

void Agent::RegisterProperties() {
	registry->Register("name", &name);
	transform.RegisterProperties(*registry);
}

void Agent::SetEditorMode(bool selected) {
	editorMode = selected;
	if (selected) {}
}

void Agent::HandleMouse(const glm::vec2& worldMouse, bool mouseDown) {
	if (editorMode) {
		if (editorMode) {
			glm::vec2 center = transform.position;
			glm::vec2 halfSize = transform.scale * 0.5f;

			glm::vec2 min = center - halfSize;
			glm::vec2 max = center + halfSize;

			bool mouseOver = worldMouse.x >= min.x && worldMouse.x <= max.x &&
				worldMouse.y >= min.y && worldMouse.y <= max.y;

			if (!mouseDown && interaction.currentAction == EditorAction::None) {
				interaction.isSelected = mouseOver;
			}

			if (mouseDown && interaction.currentAction == EditorAction::None && mouseOver) {
				interaction.currentAction = EditorAction::Drag;
				interaction.dragOffset = (worldMouse - center) / transform.scale;
			}

			if (!mouseDown) {
				interaction.currentAction = EditorAction::None;
			}

			if (interaction.currentAction == EditorAction::Drag) {
				glm::vec2 delta = (worldMouse) - (interaction.dragOffset * transform.scale);
				transform.position = delta;
			}
		}
	}
	else {
		for (auto& c : components) {
			c->HandleMouse(worldMouse, mouseDown);
		}	
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

void Agent::Serialize(std::ofstream& out) const {
	out << "[Agent " << GetType() << "]\n";
	registry->Serialize(out);

	for (auto& comp : components) {
		out << "[Component " << comp->GetType() << "]\n";
		comp->Serialize(out);
		out << "[/Component]\n";
	}

	out << "[/Agent]\n";
}

void Agent::Deserialize(std::ifstream& in) {
	std::string line;
	while (std::getline(in, line)) {
		if (line.empty()) continue;

		if (line == "[/Agent]") {
			break; // finished this agent block
		}

		// Components
		if (line.rfind("[Component", 0) == 0) {
			std::istringstream iss(line);
			std::string discard, type;
			iss >> discard >> type; // [Component SpriteComponent]

			if (type.back() == ']') {
				type.pop_back();
			}

			Component* comp = nullptr;
			if (type == "SpriteComponent")        comp = AddComponent<SpriteComponent>();
			else if (type == "SolidCollider")     comp = AddComponent<SolidCollider>();
			else if (type == "TriggerCollider")   comp = AddComponent<TriggerCollider>();
			else if (type == "HurtboxCollider")   comp = AddComponent<HurtboxCollider>();
			else if (type == "HitboxCollider")    comp = AddComponent<HitboxCollider>();
			else if (type == "StatsComponent")    comp = AddComponent<StatsComponent>();
			else if (type == "AnimatorComponent") comp = AddComponent<AnimatorComponent>();
			else if (type == "ScriptComponent") {
				ScriptManager& scriptManager = this->scene->GetScriptManager();
				comp = AddComponent<ScriptComponent>(scriptManager);
			}
			if (comp) {
				comp->Deserialize(in); // consumes until [/Component]
			}
		}
		else {
			// Allow both "key=value" and "key value"
			std::string key, value;
			auto pos = line.find('=');
			if (pos != std::string::npos) {
				key = line.substr(0, pos);
				value = line.substr(pos + 1);
			}
			else {
				std::istringstream iss(line);
				iss >> key;
				std::getline(iss, value);
				if (!value.empty() && value[0] == ' ') value.erase(0, 1); // trim leading space
			}

			if (key.empty()) continue;

			// Try registry first
			std::istringstream valStream(value);
			bool handled = false;
			for (auto& p : registry->properties) {
				if (p->name == key) {
					p->Deserialize(valStream);
					handled = true;
					break;
				}
			}
			if (handled) continue;

			// Fallback manual handling
			if (key == "position") {
				std::istringstream vs(value);
				vs >> transform.position.x >> transform.position.y;
			}
			else if (key == "scale") {
				std::istringstream vs(value);
				vs >> transform.scale.x >> transform.scale.y;
			}
			else if (key == "rotation") {
				std::istringstream vs(value);
				vs >> transform.rotation;
			}
		}
	}
}

void Agent::DrawImGui() {
	registry->DrawImGui();
	ImGui::Separator();

	int componentID = 0;
	for (const auto& comp : components) {
		ImGui::PushID(componentID++);
		ImGui::NewLine();
		ImGui::Separator();
		comp->DrawImGui();
		ImGui::PopID();
	}

	if (ImGui::Button("Add Component")) {
		ImGui::OpenPopup("AddComponentPopup");
	}

	if (ImGui::BeginPopup("AddComponentPopup")) {
		if (ImGui::MenuItem("Sprite")) AddComponent<SpriteComponent>(DEFAULT_SPRITE);
		if (ImGui::BeginMenu("Collider")) {
			if (ImGui::MenuItem("Solid")) AddComponent<SolidCollider>();
			if (ImGui::MenuItem("Hitbox")) AddComponent<HitboxCollider>();
			if (ImGui::MenuItem("Hurtbox")) AddComponent<HurtboxCollider>();
			if (ImGui::MenuItem("Trigger")) AddComponent<TriggerCollider>();
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("AnimatorComponent")) AddComponent<AnimatorComponent>();
		else if (type == "ScriptComponent") {
			ScriptManager& scriptManager = this->scene->GetScriptManager();
			AddComponent<ScriptComponent>(scriptManager);
		}
		if (ImGui::MenuItem("Stats")) AddComponent<StatsComponent>();
		ImGui::EndPopup();
	}
}

void Agent::SaveToFile(const std::string& filePath) const {
	std::ofstream out(filePath);
	if (!out.is_open()) return;
	Serialize(out);
}

void Agent::SaveToFile(std::ofstream& out) const {
	Serialize(out);
}

void Agent::LoadFromFile(std::ifstream& in) {
	if (!in) return;

	components.clear();

	Deserialize(in);
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