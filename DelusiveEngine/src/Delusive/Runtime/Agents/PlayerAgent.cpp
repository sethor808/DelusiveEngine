#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Talismans/DelusiveTalismans.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Player/DelusiveInventory.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <iostream>

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
}

void PlayerAgent::LinkScene(Scene* scene) {
    sceneLink = scene;
    inventoryLink = sceneLink->GetInventoryLink();
	LoadFromInventory();
}

std::string PlayerAgent::GetType() const{
    return "PlayerAgent";
}

void PlayerAgent::RegisterProperties() {
	Agent::RegisterProperties();
	registry->Register("weaponSlots", &weaponSlots);
	registry->Register("swapCooldown", &swapCooldown);
}

void PlayerAgent::HandleInput(const PlayerInputState& input) {
    // Movement logic
    bool dodgePressed = (input.dodgePressed && !prevInput.dodgePressed);

    if (dodgePressed) {
        dodgeBufferTimer = 0.15f;
        dodgeDir = input.moveDir;
    }

	//Ticks during the lock are dropped, not queued, so a fast flick can't slide past a slot
	if (input.swapDir != 0 && swapLockTimer <= 0.0f) {
		SwapWeapon(input.swapDir);
	}

    prevInput = input;
}

void PlayerAgent::Update(float deltaTime) {
    if (CheckIfDead()) {
        //TODO: Die
    }

	if (swapLockTimer > 0.0f) {
		swapLockTimer -= deltaTime;
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

void PlayerAgent::SwapWeapon(int dir) {
	if (weaponSlots <= 1) return;

	weaponIndex = (weaponIndex + dir + weaponSlots) % weaponSlots;
	swapLockTimer = swapCooldown;
	std::cout << "[PlayerAgent] Weapon slot " << weaponIndex << std::endl; //TODO: Equip the slot's weapon once weapons exist
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
	//Talismans work as a stack: a hit only damages the first unbroken one
    for (auto& talisman : talismans) {
        if (talisman->GetIsBroken()) {
            continue;
        }

        if (talisman->TakeDamage()) {
            //Activate consume passive here
            talisman->OnConsume(); //TODO: Make this do something
        }
		return;
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

void PlayerAgent::TriggerInvul(float duration) {
	invulTimer = duration; //TODO: Add more effects to make this really feel like an invul
}

void PlayerAgent::ResetStats() {
    modified = base; //TODO: Reapply talisman effects after sync
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
}