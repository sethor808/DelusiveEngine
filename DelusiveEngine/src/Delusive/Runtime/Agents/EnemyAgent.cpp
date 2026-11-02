#include <Delusive/Runtime/Agents/EnemyAgent.h>
#include <Delusive/Runtime/Components/DelusiveComponents.h>
#include <Delusive/Runtime/Collision/HitData.h>

EnemyAgent::EnemyAgent(DelusiveInstance& instance)
    : Agent(instance)
{
    SetName("New EnemyAgent");
    SetScale({ 1.0f, 1.0f });
    velocity = { 0.0f, 0.0f };

    RegisterProperties();
}

void EnemyAgent::RegisterProperties() {
    Agent::RegisterProperties();

    registry->Register("moveSpeed", &moveSpeed);
    registry->Register("damping", &damping);
    registry->Register("maxHealth", &maxHealth);
}

std::string EnemyAgent::GetType() const{
    return "EnemyAgent";
}

void EnemyAgent::Update(float deltaTime) {
    //if (logicScript) {
      //  logicScript->Update(deltaTime);
    //}

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
    /*
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
    */
}

void EnemyAgent::SetTarget(Agent* target) {
    this->target = target;
}

void EnemyAgent::HitEvent(const HitData& hit) {
	if (IsDestroyed()) return;

	damageTaken += hit.damage;
	if (GetHealth() <= 0) Destroy();
}
