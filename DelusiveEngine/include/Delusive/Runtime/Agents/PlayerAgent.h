#pragma once
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Utils/DelusiveUtils.h>
#include <Delusive/Runtime/Player/PlayerStats.h>
#include <Delusive/Runtime/Player/PlayerInputState.h>
#include <Delusive/Runtime/Talismans/Talisman.h> //Owned by unique_ptr, so it must be complete

class Talisman;
class DelusiveInventory;

class PlayerAgent : public Agent {
public:
	explicit PlayerAgent(DelusiveInstance&);
	PlayerAgent() = delete;

    void LinkScene(Scene* scene) override;

	void Update(float deltaTime) override;
	void HandleInput(const PlayerInputState&);
	std::string GetType() const override;
	void RegisterProperties() override;

	void ApplyKnockback(const glm::vec2&, float);

	void HitEvent(const HitData&) override;
	void LandHitEvent(const HitData&) override;
	void ModifyOutgoingHit(HitData&) const override;
	float GetHitboxScale() const override { return modified.hitboxScale; }
	//Hits every agent off the player's team within radius, e.g. talisman explosions
	void HitAround(float radius, int damage);
	const std::vector<std::unique_ptr<Talisman>>& GetTalismans() { return talismans; }
	//First unbroken talisman: the one the next hit damages
	Talisman* GetActiveTalisman() const;

	//Keeps the longer of the current and new window
	void TriggerInvul(float);
	bool IsInvulnerable() const { return invulTimer > 0.0f; }
	const PlayerStats& GetStats() const { return modified; }
	//Base stats plus talisman hooks; call whenever the stack changes
	void RecalculateStats();

	//InventoryAccess
	//Full HP copies of the equipped talismans
	void LoadFromInventory();
	DelusiveInventory* GetInventory() { return inventoryLink; }

private:
	PlayerStats base;
	PlayerStats modified;

	//HP Management
	std::vector<std::unique_ptr<Talisman>> talismans;

	glm::vec2 velocity = { 0.0f, 0.0f };
	glm::vec2 impulse = { 0.0f, 0.0f };
	glm::vec2 dodgeDir = { 0.0f, 0.0f };

	//Dodge variables
	bool dodging = false;

	float dodgeTimer = 0.0f;

	float inputLockTimer = 0.0f;
	float dodgeBufferTimer = 0.0f;
	float invulTimer = 0.0f;

	//Weapon slots: a swap locks further swaps for swapCooldown so one wheel flick moves one slot
	int weaponIndex = 0;
	int weaponSlots = 2;
	float swapCooldown = 0.12f;
	float swapLockTimer = 0.0f;

	//For input control tracking
	PlayerInputState prevInput;

	//Inventory Management
	DelusiveInventory* inventoryLink = nullptr;

	//PlayerState Helpers
	bool CheckIfDead();

	//Player Control Functions
	void StartDodge(const glm::vec2&);
	void HandleDodge(float);
	void HandleMovement(float deltaTime);
	void SwapWeapon(int);
};