#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Talismans/DelusiveTalismans.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Player/DelusiveInventory.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <iostream>
#include <cmath>
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

	if (invulTimer > 0.0f) {
		invulTimer -= deltaTime;
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
    impulse = dir * modified.moveSpeed * modified.dodgeSpeedScale;

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
	if (!active || IsInvulnerable()) return;

	for (auto& talisman : talismans) {
		if (!talisman->GetIsBroken()) talisman->HitEvent(*this, hit);
	}

	//Talismans work as a stack: a hit only damages the first unbroken one
	if (active->TakeDamage(hit.damage)) {
		active->OnConsume(*this);
		RecalculateStats();
	}

	TriggerInvul(modified.invulDuration);
}

void PlayerAgent::LandHitEvent(const HitData& hit) {
	for (auto& talisman : talismans) {
		if (!talisman->GetIsBroken()) talisman->LandHitEvent(*this, hit);
	}
}

void PlayerAgent::ModifyOutgoingHit(HitData& hit) const {
	hit.damage = static_cast<int>(std::lround(hit.damage * modified.damageMultiplier));
}

void PlayerAgent::HitAround(float radius, int damage) {
	if (!sceneLink) return;

	for (auto& agent : sceneLink->GetAgents()) {
		if (agent.get() == this || (team != 0 && agent->GetTeam() == team)) continue;

		const glm::vec2 offset = agent->GetTransform().position - transform.position;
		const float distance = glm::length(offset);
		if (distance > radius) continue;

		HitData hit;
		hit.attacker = this;
		hit.target = agent.get();
		hit.damage = damage;
		hit.parryable = false;
		if (distance > 0.0f) hit.direction = offset / distance;
		agent->HitEvent(hit);
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
	invulTimer = std::max(invulTimer, duration); //TODO: Add more effects to make this really feel like an invul
}

void PlayerAgent::RecalculateStats() {
	modified = base;

	for (const auto& talisman : talismans) {
		if (talisman->GetIsBroken()) talisman->ApplyWhileBroken(modified);
		else talisman->ApplyPassive(modified);
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