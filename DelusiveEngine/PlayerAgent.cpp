#include "PlayerAgent.h"
#include "Renderer.h"

PlayerAgent::PlayerAgent(const std::string& agentName) {
	SetName(agentName);
	SetScale({1.0f, 1.0f});
	velocity = { 0.0f, 0.0f };
}

std::string PlayerAgent::GetType() const{
    return "PlayerAgent";
}


void PlayerAgent::HandleInput(const PlayerInputState& input) {
    // Movement logic
    bool dodgePressed = (input.dodgePressed && !prevInput.dodgePressed);

    if (dodgePressed) {
        dodgeBufferTimer = 0.15;
        dodgeDir = input.moveDir;
    }

    prevInput = input;
}

void PlayerAgent::Update(float deltaTime) {
    // Input lock countdown
    if (inputLockTimer > 0.0f) {
        inputLockTimer -= deltaTime;
    }

    // Input buffers
    // TODO: Streamline this to its own code
    if (!dodging && dodgeBufferTimer > 0.0f && inputLockTimer <= 0.0f)
    {
        if (glm::length(dodgeDir) > 0.0f) {
            StartDodge(glm::normalize(dodgeDir));
            dodgeBufferTimer = 0.0f; // consume buffer
        }
    }

    if (dodging) {
        HandleDodge(deltaTime);
    }
    else {
        HandleMovement(deltaTime);
    }

    // If dodging, override with dodge impulse
    glm::vec2 finalVelocity = velocity + impulse;

    // Move player
    transform.position += finalVelocity * deltaTime;

    // Update all components
    for (auto& comp : components) {
        comp->Update(deltaTime);
    }
}

void PlayerAgent::HandleMovement(float deltaTime) {
    glm::vec2 targetVel = glm::vec2(0.0f);

    // Normal WASD movement
    if (glm::length(prevInput.moveDir) > 0.0f) {
        glm::vec2 dir = glm::normalize(prevInput.moveDir);
        velocity = dir * moveSpeed;
    }

    //Apply acceleration
    float accelRate = (glm::length(targetVel) > 0.0f) ? acceleration : deceleration;
    velocity = glm::mix(velocity, targetVel, accelRate * deltaTime);

    float decayRate = 8.0f; // higher = stops faster
    impulse *= std::max(0.0f, 1.0f - decayRate * deltaTime);
    if (glm::length(impulse) < 0.01f) {
        impulse = glm::vec2(0.0f);
    }
}

void PlayerAgent::StartDodge(const glm::vec2& dir) {
    dodging = true;
    dodgeTimer = dodgeDuration;
    impulse = dir * dodgeStrength;
    inputLockTimer = dodgeDuration;
}

void PlayerAgent::HandleDodge(float deltaTime) {
    // Process dodge buffering
    if (dodgeBufferTimer > 0.0f) {
        dodgeBufferTimer -= deltaTime;
    }

    dodgeTimer -= deltaTime;
    if (dodgeTimer <= 0.0f) {
        dodging = false;
        impulse = glm::vec2(0.0f);
    }
}

void PlayerAgent::ApplyKnockback(const glm::vec2& dir, float strength) {
    impulse = glm::normalize(dir) * strength;
    //dodging = false;
    inputLockTimer = 0.1f;
}

void PlayerAgent::DrawImGui() {
    ImGui::Text("PlayerAgent");
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