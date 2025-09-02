#include "EnemyAgent.h"
EnemyAgent::EnemyAgent(const std::string& agentName) {
    SetName(agentName);
    SetScale({ 1.0f, 1.0f });
    velocity = { 0.0f, 0.0f };

    RegisterProperties();
}

void EnemyAgent::RegisterProperties() {
    Agent::RegisterProperties();

    registry.Register("scriptName", &scriptName);
    registry.Register("moveSpeed", &moveSpeed);
    registry.Register("damping", &damping);
}

std::string EnemyAgent::GetType() const{
    return "EnemyAgent";
}

std::unique_ptr<Agent> EnemyAgent::Clone() const {
    auto copy = std::make_unique<EnemyAgent>(this->GetName());  // or however the agent is constructed

    // Copy transform and basic properties
    if (!scriptName.empty()) {
        copy->logicScript = ScriptRegistry::Instance().Create(scriptName);
    }

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

void EnemyAgent::DrawImGui() {
    Agent::DrawImGui();

    auto names = ScriptRegistry::Instance().GetNames();
    int currentIndex = 0;
    for (int i = 0; i < (int)names.size(); i++) {
        if (names[i] == scriptName) { currentIndex = i; break; }
    }

    if (ImGui::Combo("Script", &currentIndex,
        [](void* data, int idx, const char** out_text) {
            auto& vec = *reinterpret_cast<std::vector<std::string>*>(data);
            if (idx < 0 || idx >= (int)vec.size()) return false;
            *out_text = vec[idx].c_str();
            return true;
        },
        (void*)&names, (int)names.size())) {
        SetScript(names[currentIndex]);
    }
}

void EnemyAgent::OnHit() {

}

void EnemyAgent::SetScript(const std::string& _scriptName) {
    scriptName = _scriptName;
    logicScript = ScriptRegistry::Instance().Create(name);
}

void EnemyAgent::SetTarget(Agent* target) {
    this->target = target;
}