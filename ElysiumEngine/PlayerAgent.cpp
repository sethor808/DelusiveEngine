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

std::unique_ptr<Agent> PlayerAgent::Clone() const {
    auto copy = std::make_unique<PlayerAgent>(this->GetName());  // or however the agent is constructed

    // Copy transform and basic properties
    copy->SetPosition(this->GetTransform().position);
    copy->SetRotation(this->GetTransform().rotation);
    copy->SetScale(this->GetTransform().scale);
    copy->SetName(this->GetName());

    // Deep copy each component
    for (const auto& comp : this->GetComponents()) {
        if (comp) {
            std::unique_ptr<Component> clone = comp->Clone();
            if (clone) {
                copy->AddRawComponent(std::move(clone));  // Assumes internal AddRawComponent
            }
        }
    }

    return copy;
}

void PlayerAgent::Draw(const glm::mat4& projection) const {
	for (const auto& comp : this->GetComponents()) {
		comp->Draw(projection);
	}
}