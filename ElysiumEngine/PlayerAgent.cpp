#include "PlayerAgent.h"
#include "Renderer.h"

PlayerAgent::PlayerAgent(const std::string& agentName) {
	SetName(agentName);
	SetScale({1.0f, 1.0f});
	velocity = { 0.0f, 0.0f };
}

void PlayerAgent::Update(float deltaTime) {
	transform.position += velocity * deltaTime;
	/*
	sprite->SetPosition(transform.position.x, transform.position.y);
	sprite->SetRotation(transform.rotation);
	sprite->SetScale(transform.scale.x, transform.scale.y);
	*/
}

void PlayerAgent::Draw(const glm::mat4& projection) const {
	for (const auto& comp : this->GetComponents()) {
		comp->Draw(projection);
	}
}