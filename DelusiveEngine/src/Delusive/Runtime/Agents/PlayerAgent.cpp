#include <Delusive/Runtime/Agents/PlayerAgent.h>
#include <Delusive/Runtime/Talismans/DelusiveTalismans.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Player/DelusiveInventory.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/Core/DelusiveFactory.h>
#include <Delusive/Runtime/Core/DelusiveRegistry.h>
#include <Delusive/Runtime/Agents/EnemyAgent.h>
#include <Delusive/Runtime/Components/SpriteComponent.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <iostream>
#include <cmath>
#include <Delusive/Runtime/Collision/HitData.h>

namespace {
	struct ChainStep {
		float windup;
		float hold;
		float recovery;
		int damage;
	};

	//Seconds; the third hit lingers so a weapon swap has room to cancel it
	constexpr ChainStep LIGHT_CHAIN[] = {
		{ 0.05f, 0.12f, 0.15f, 1 },
		{ 0.05f, 0.12f, 0.15f, 1 },
		{ 0.08f, 0.15f, 0.45f, 2 },
	};
	constexpr int LIGHT_CHAIN_LENGTH = 3;
	constexpr float ATTACK_REACH = 0.8f;
	constexpr float ATTACK_BUFFER = 0.2f;
}

PlayerAgent::PlayerAgent(DelusiveInstance& instance)
    : Agent(instance)
{
    SetName("New PlayerAgent");
	SetScale({1.0f, 1.0f});
	velocity = { 0.0f, 0.0f };
	lightChainSprites = {
		SPRITE_FOLDER "BasicDude/BasicDude2.png",
		SPRITE_FOLDER "BasicDude/BasicDude3.png",
		SPRITE_FOLDER "BasicDude/BasicDude4.png"
	};

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
	registry->Register("lightChainSprites", &lightChainSprites);
}

void PlayerAgent::HandleInput(const PlayerInputState& input) {
    if (input.dodgePressed) {
        dodgeBufferTimer = 0.15f;
        dodgeDir = input.moveDir;
    }

	if (glm::length(input.moveDir) > 0.0f) facing = glm::normalize(input.moveDir);
	if (input.lightAttackPressed) attackBufferTimer = ATTACK_BUFFER;

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

	if (attackBufferTimer > 0.0f) {
		attackBufferTimer -= deltaTime;
	}

    // Input lock countdown
    if (inputLockTimer > 0.0f) {
        inputLockTimer -= deltaTime;
    }

    // Input buffers
    // TODO: Streamline this to its own code
	const bool attackLocked = chainStep >= 0 && !IsAttackRecovering();
	if (!dodging && dodgeBufferTimer > 0.0f && inputLockTimer <= 0.0f && !attackLocked) {
        if (glm::length(dodgeDir) > 0.0f) {
            StartDodge(glm::normalize(dodgeDir));
            dodgeBufferTimer = 0.0f; // consume buffer
        }
    }

	if (chainStep >= 0) {
		UpdateAttack(deltaTime);
	}
	else if (!dodging && attackBufferTimer > 0.0f && inputLockTimer <= 0.0f) {
		StartAttackStep(0);
	}

    if (dodging) {
        HandleDodge(deltaTime);
    }
    else {
		//Attacks root the player in place
		HandleMovement(chainStep >= 0 ? glm::vec2(0.0f) : prevInput.moveDir, deltaTime);
    }

    //std::cout << "[PlayerAgent] Current Position: " << transform.position.x << ", " << transform.position.y << std::endl;

    // If dodging, override with dodge impulse
    glm::vec2 finalVelocity = velocity + impulse;

    // Move player
    transform.position += finalVelocity * deltaTime;

    Agent::Update(deltaTime);
}

void PlayerAgent::HandleMovement(const glm::vec2& moveDir, float deltaTime) {
    glm::vec2 targetVel = glm::vec2(0.0f);

    // Normal WASD movement
    if (glm::length(moveDir) > 0.0f) {
        glm::vec2 dir = glm::normalize(moveDir);
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
	if (IsAttackRecovering()) EndAttack();
	std::cout << "[PlayerAgent] Weapon slot " << weaponIndex << std::endl; //TODO: Equip the slot's weapon once weapons exist
}

void PlayerAgent::StartAttackStep(int step) {
	SpriteComponent* sprite = GetComponentOfType<SpriteComponent>();
	if (sprite && chainStep < 0) idleSprite = sprite->GetTexturePath();
	if (sprite && step < static_cast<int>(lightChainSprites.size())) sprite->SetTexturePath(lightChainSprites[step]);

	chainStep = step;
	attackTimer = 0.0f;
	attackBufferTimer = 0.0f;
	hitLanded = false;
}

void PlayerAgent::UpdateAttack(float deltaTime) {
	const ChainStep& step = LIGHT_CHAIN[chainStep];
	attackTimer += deltaTime;

	if (!hitLanded && attackTimer >= step.windup) {
		hitLanded = true;
		for (Agent* target : FetchTargetsWithin(transform.position + facing * ATTACK_REACH, ATTACK_REACH)) {
			//Walls and cameras sit in range too; only enemies count as a landed hit
			if (!dynamic_cast<EnemyAgent*>(target)) continue;

			HitData hit;
			hit.attacker = this;
			hit.target = target;
			hit.damage = step.damage;
			hit.direction = facing;
			ModifyOutgoingHit(hit);
			target->HitEvent(hit);
			LandHitEvent(hit);
		}
	}

	if (!IsAttackRecovering()) return;

	if (attackBufferTimer > 0.0f && chainStep + 1 < LIGHT_CHAIN_LENGTH) {
		StartAttackStep(chainStep + 1);
	}
	else if (attackTimer >= step.windup + step.hold + step.recovery) {
		EndAttack();
	}
}

void PlayerAgent::EndAttack() {
	if (chainStep < 0) return;
	chainStep = -1;

	SpriteComponent* sprite = GetComponentOfType<SpriteComponent>();
	if (sprite && !idleSprite.empty()) sprite->SetTexturePath(idleSprite);
}

bool PlayerAgent::IsAttackRecovering() const {
	if (chainStep < 0) return false;
	const ChainStep& step = LIGHT_CHAIN[chainStep];
	return attackTimer >= step.windup + step.hold;
}

void PlayerAgent::StartDodge(const glm::vec2& dir) {
	EndAttack();
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
	for (Agent* target : FetchTargetsWithin(transform.position, radius)) {
		const glm::vec2 offset = target->GetTransform().position - transform.position;
		const float distance = glm::length(offset);

		HitData hit;
		hit.attacker = this;
		hit.target = target;
		hit.damage = damage;
		hit.parryable = false;
		if (distance > 0.0f) hit.direction = offset / distance;
		target->HitEvent(hit);
	}
}

std::vector<Agent*> PlayerAgent::FetchTargetsWithin(const glm::vec2& centre, float radius) const {
	std::vector<Agent*> targets;
	if (!sceneLink) return targets;

	for (auto& agent : sceneLink->GetAgents()) {
		if (agent.get() == this || (team != 0 && agent->GetTeam() == team)) continue;
		if (glm::length(agent->GetTransform().position - centre) <= radius) targets.push_back(agent.get());
	}
	return targets;
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