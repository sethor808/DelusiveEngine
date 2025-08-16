#include "EnemyAgent.h"
EnemyAgent::EnemyAgent(const std::string& agentName) {
    SetName(agentName);
    SetScale({ 1.0f, 1.0f });
    velocity = { 0.0f, 0.0f };
}

std::string EnemyAgent::GetType() const{
    return "EnemyAgent";
}

std::unique_ptr<Agent> EnemyAgent::Clone() const {
    auto copy = std::make_unique<EnemyAgent>(this->GetName());  // or however the agent is constructed

    // Copy transform and basic properties
    copy->SetScript(logicScript->Clone());
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

void EnemyAgent::Update(float deltaTime) {
    if (logicScript) {
        logicScript->Update(this, deltaTime);
    }

    for (auto& comp : GetComponents()) {
        comp->Update(deltaTime);
    }
}

void EnemyAgent::Draw(const glm::mat4& projection) const {
    for (const auto& comp : this->GetComponents()) {
        comp->Draw(projection);
    }
}

void EnemyAgent::OnHit() {

}

void EnemyAgent::SetScript(std::unique_ptr<EnemyScript> script) {
    logicScript = std::move(script);
}

void EnemyAgent::SetTarget(Agent* target) {
    this->target = target;
}