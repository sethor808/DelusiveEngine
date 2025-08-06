#include "EnvironmentAgent.h"

EnvironmentAgent::EnvironmentAgent(const std::string& agentName) {
	SetName(agentName);
	SetScale({ 1.0f, 1.0f });
}

std::unique_ptr<Agent> EnvironmentAgent::Clone() const {
    auto copy = std::make_unique<EnvironmentAgent>(this->GetName());

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

void EnvironmentAgent::Update(float deltaTime) {

}

void EnvironmentAgent::Draw(const glm::mat4& projection) const {
    for (const auto& comp : this->GetComponents()) {
        comp->Draw(projection);
    }
}