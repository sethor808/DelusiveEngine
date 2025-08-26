#include "EnemyAgent.h"
EnemyAgent::EnemyAgent(const std::string& agentName) {
    SetName(agentName);
    SetScale({ 1.0f, 1.0f });
    velocity = { 0.0f, 0.0f };

    RegisterProperties();
}

void EnemyAgent::RegisterProperties() {
    Agent::RegisterProperties();

    registry.Register("moveSpeed", &moveSpeed);
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

void EnemyAgent::DrawImGui() {
    ImGui::Text("EnemyAgent");
    ImGui::Separator();

    ImGui::Text("Name");
    ImGui::SameLine();

    char nameBuffer[64];

    // Copy current name into buffer
    strncpy_s(nameBuffer, GetName().c_str(), sizeof(nameBuffer));

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

    if (ImGui::InputText("##agentName", nameBuffer, sizeof(nameBuffer), flags)) {
        // This triggers only when Enter is pressed
        if (nameBuffer[0] == '\0') {
            strncpy_s(nameBuffer, GetName().c_str(), sizeof(nameBuffer));
        }
        else {
            this->SetName(nameBuffer);
        }
    }

    ImGui::Separator();
    ImGui::Text("Transform");
    ImGui::Text("Position: ");
    ImGui::SameLine();
    glm::vec2 pos = transform.position;
    if (ImGui::DragFloat2("##position", glm::value_ptr(pos), 1.0f)) {
        transform.position = pos;
    }

    ImGui::Text("Rotation: ");
    ImGui::SameLine();
    float rot = transform.rotation;
    if (ImGui::DragFloat("##rotation", &rot, 0.1f)) {
        transform.rotation = rot;
    }

    ImGui::Text("Scale:    ");
    ImGui::SameLine();
    glm::vec2 scale = transform.scale;
    if (ImGui::DragFloat2("##scale", glm::value_ptr(scale), 0.1f)) {
        transform.scale = scale;
    }

    int componentID = 0;
    for (const auto& comp : GetComponents()) {
        ImGui::PushID(componentID++);
        ImGui::NewLine();
        ImGui::Separator();
        comp->DrawImGui();
        ImGui::PopID();
    }

    components.erase(
        std::remove_if(components.begin(), components.end(),
            [](const std::unique_ptr<Component>& comp) {
                return comp->ToDelete();
            }),
        components.end());

    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup")) {
        if (ImGui::MenuItem("Sprite")) AddComponent<SpriteComponent>("assets/sprites/star.jpg");
        if (ImGui::BeginMenu("Collider")) {
            if (ImGui::MenuItem("Solid")) AddComponent<SolidCollider>();
            if (ImGui::MenuItem("Hitbox")) AddComponent<HitboxCollider>();
            if (ImGui::MenuItem("Hurtbox")) AddComponent<HurtboxCollider>();
            if (ImGui::MenuItem("Trigger")) AddComponent<TriggerCollider>();
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("AnimatorComponent")) AddComponent<AnimatorComponent>();
        if (ImGui::MenuItem("Stats")) AddComponent<StatsComponent>();
        ImGui::EndPopup();
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