#include "PlayerAgent.h"
#include "DelusiveTalismans.h"
#include "DelusiveRenderer.h"
#include "DelusiveInventory.h"

PlayerAgent::PlayerAgent(DelusiveRenderer& renderer)
    : Agent(renderer), inventory(std::make_unique<DelusiveInventory>())
{
    SetName("New PlayerAgent");
	SetScale({1.0f, 1.0f});
	velocity = { 0.0f, 0.0f };

	int numTalismans = 5; // Experimental control variable
    for (int i = 0; i < numTalismans; ++i) {
        talismans.push_back(std::make_unique<BasicTalisman>());
    }
}

std::string PlayerAgent::GetType() const{
    return "PlayerAgent";
}

void PlayerAgent::HandleInput(const PlayerInputState& input) {
    // Movement logic
    bool dodgePressed = (input.dodgePressed && !prevInput.dodgePressed);

    if (dodgePressed) {
        dodgeBufferTimer = 0.15f;
        dodgeDir = input.moveDir;
    }

    prevInput = input;
}

void PlayerAgent::Update(float deltaTime) {
    if (CheckIfDead()) {
        //TODO: Die
    }

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

    std::cout << "[PlayerAgent] Current Position: " << transform.position.x << ", " << transform.position.y << std::endl;

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
        velocity = dir * modified.moveSpeed;
    }

    //Apply acceleration
    float accelRate = (glm::length(targetVel) > 0.0f) ? modified.acceleration : modified.deceleration;
    velocity = glm::mix(velocity, targetVel, accelRate * deltaTime);

    float decayRate = 8.0f; // higher = stops faster
    impulse *= std::max(0.0f, 1.0f - decayRate * deltaTime);
    if (glm::length(impulse) < 0.01f) {
        impulse = glm::vec2(0.0f);
    }
}

void PlayerAgent::StartDodge(const glm::vec2& dir) {
    dodging = true;
    dodgeTimer = modified.dodgeDuration;

    velocity = velocity * 0.5f;
    impulse = dir * modified.dodgeStrength;

    inputLockTimer = modified.dodgeDuration;
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

void PlayerAgent::TakeDamage() {
    for (auto& talisman : talismans) {
        if (talisman->GetIsBroken()) {
            continue;
        }

        if (talisman->TakeDamage()) {
            //Activate consume passive here
            talisman->OnConsume(); //TODO: Make this do something
        }
    }
}

bool PlayerAgent::CheckIfDead() {
    if (talismans.empty()) {
        return true;
    }

    //Talismans are always added with push_back
    //Last talisman is always the final one to break
    return talismans.back()->GetIsBroken();
}

void PlayerAgent::ApplyKnockback(const glm::vec2& dir, float strength) {
    impulse = glm::normalize(dir) * strength;
    //dodging = false;
    inputLockTimer = 0.1f;
}

std::unique_ptr<Agent> PlayerAgent::Clone(Scene* scene) const {
    auto copy = std::make_unique<PlayerAgent>(renderer);
    CloneBaseProperties(copy.get(), scene);
    return copy;
}

void PlayerAgent::Draw(const glm::mat4& projection) const {
	for (const auto& comp : this->GetComponents()) {
		comp->Draw(projection);
	}
}

void PlayerAgent::TriggerInvul(float duration) {
	invulTimer = duration; //TODO: Add more effects to make this really feel like an invul
}

void PlayerAgent::ResetStats() {
    modified = base; //TODO: Reapply talisman effects after sync
}

void PlayerAgent::EquipTalisman(int index, Talisman* talisman) {
    if (index < 0 || index >= static_cast<int>(talismans.size())) return;
    
    //TODO: Properly equip talisman
	//inventory->EquipTalisman(index, talisman);
}