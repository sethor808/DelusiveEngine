#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Talismans/DelusiveTalismans.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Player/DelusiveInventory.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Collision/HitData.h>

PlayerAgent::PlayerAgent(DelusiveInstance& instance)
    : Agent(instance)
{
    SetName("New PlayerAgent");
	SetScale({1.0f, 1.0f});
	velocity = { 0.0f, 0.0f };

	int numTalismans = 5; // Experimental control variable
    for (int i = 0; i < numTalismans; ++i) {
        talismans.push_back(std::make_unique<BasicTalisman>());
    }

    RegisterProperties();
	RecalculateStats();
}

void PlayerAgent::LinkScene(Scene* scene) {
    sceneLink = scene;
    inventoryLink = sceneLink->GetInventoryLink();
	LoadFromInventory();
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

    //std::cout << "[PlayerAgent] Current Position: " << transform.position.x << ", " << transform.position.y << std::endl;

    // If dodging, override with dodge impulse
    glm::vec2 finalVelocity = velocity + impulse;

    // Move player
    transform.position += finalVelocity * deltaTime;

    Agent::Update(deltaTime);
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

void PlayerAgent::HitEvent(const HitData& hit) {
	Talisman* active = GetActiveTalisman();
	if (!active) return;

	for (auto& talisman : talismans) {
		if (!talisman->GetIsBroken()) talisman->HitEvent(*this, hit);
	}

	//Talismans work as a stack: a hit only damages the first unbroken one
	if (active->TakeDamage(hit.damage)) {
		active->OnConsume(*this);
		RecalculateStats();
	}
}

void PlayerAgent::LandHitEvent(const HitData& hit) {
	for (auto& talisman : talismans) {
		if (!talisman->GetIsBroken()) talisman->LandHitEvent(*this, hit);
	}
}

Talisman* PlayerAgent::GetActiveTalisman() const {
	for (const auto& talisman : talismans) {
		if (!talisman->GetIsBroken()) return talisman.get();
	}
	return nullptr;
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

void PlayerAgent::TriggerInvul(float duration) {
	invulTimer = duration; //TODO: Add more effects to make this really feel like an invul
}

void PlayerAgent::RecalculateStats() {
	modified = base;

	for (const auto& talisman : talismans) {
		if (!talisman->GetIsBroken()) talisman->ApplyPassive(modified);
	}

	if (Talisman* active = GetActiveTalisman()) active->ApplyWhileActive(modified);
}

void PlayerAgent::LoadFromInventory() {
	if (!inventoryLink) return;

	//Copies, so run damage never touches the inventory
	std::vector<std::unique_ptr<Talisman>> loadout;
	for (Talisman* equipped : inventoryLink->GetEquippedTalismans()) {
		if (!equipped) continue;
		auto talisman = DelusiveFactory<Talisman>::Create(equipped->GetType(), instance);
		if (talisman) loadout.push_back(std::move(talisman));
	}

	//Empty keeps the defaults for scenes without an equip screen
	if (!loadout.empty()) talismans = std::move(loadout);
	RecalculateStats();
}