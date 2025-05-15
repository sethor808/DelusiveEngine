#include "PlayerAgent.h"

PlayerAgent::PlayerAgent(const char* texturePath, const std::string& agentName)
	: sprite(std::make_unique<Sprite>(texturePath)) {
	SetName(agentName);
	SetScale({1.0f, 1.0f});
	velocity = { 0.0f, 0.0f };
}

void PlayerAgent::Update(float deltaTime) {
	transform.position += velocity * deltaTime;
	sprite->SetPosition(transform.position.x, transform.position.y);
	sprite->SetRotation(transform.rotation);
	sprite->SetScale(transform.scale.x, transform.scale.y);
}

void PlayerAgent::Draw(const glm::mat4& projection) const {
	sprite->Draw(projection);
}