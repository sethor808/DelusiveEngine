# Play Demo Roadmap

Last updated 2026-10-21. This is the plan for the play demo on `editor-overhaul-wip`, with the decisions made since it was first written on 2026-10-12.

## The demo

The play demo has four pillars, plus the editor tools to author them:

- **Weapons:** multiple weapons to swap between, each with light and heavy combos and input buffering.
- **Talismans:** HP as a customizable stack of talismans with Passive, While-Active and OnConsume effects.
- **Enemies:** simple enemies with one-sprite windups: ranged, melee, suicide bomber, an elite and a boss.
- **Blessings:** Kami-granted powers, 3 equipped by default, triggered by meeting conditions. They replace ability cooldowns and reward the playstyle the player picks.

## Status

| Pillar | Done | Status |
| --- | --- | --- |
| Talismans | ~65% | Inventory, EquipScreen, save/load and HP-as-talismans work. Hooks and the Speed talismans are in draft PR #4. |
| Weapons | ~10% | Wheel swap and M1/M2 input are in draft PR #5. No combos, buffer or weapon data yet. |
| Enemies | ~15% | `BasicFollow` chases. `BasicRanged` attack is a TODO. No melee, bomber, elite, boss or windups. |
| Blessings | 0% | No hit counter, parry, condition system or mode override. |

Percentages are rough estimates from reading the code, not measurements.

## Milestone order

Start with the hit event and talisman hooks: they are small, touch code that already works, and every later phase reads the same `HitData`.

| Phase | Work | Needs | State |
| --- | --- | --- | --- |
| 1. Foundations | `HitData`, `HitEvent()`/`LandHitEvent()`, talisman hooks, Speed talismans | Nothing | Draft PR #4 |
| 2. Light and heavy | Animator frame effects, runtime hitbox pool, input buffer, combo builder | 1 | M1/M2 input in draft PR #5 |
| 3. Hit counter and parry | `HitLanded` event, parry box, resolve step, stagger and hit-stop | 2 | Not started |
| 4. Enemies | Shared windup loop, five enemy types | 2 (elite and boss also need 3) | Not started |
| 5. Weapons | Weapon = combo set, two slots, wheel swap | 2 | Swap input in draft PR #5 |
| 6. Blessings | Three slots, Shadowstep, Retribution, one passive | 3 and 5 | Not started |

Phases 4 and 5 can run side by side once phase 2 lands. Blessings come last because they listen to hits, parries and movement from every earlier phase.

## Combat core

Light and heavy attacks driven by a data-built combo tree with a short input buffer, plus a structured hit event, unblock everything else in the demo.

Inputs: light attack is M1 (or J), heavy is M2 (or K). The keyboard binds are secondary. PR #5 wires the mouse buttons into `PlayerInputState`; `PlayerAgent` does not act on light or heavy yet. Controller input is wanted eventually.

### 1. Hit event (phase 1, PR #4)

One struct that every system can read: talisman triggers, the hit counter, parry and the parry-explosion Blessing all need to know who hit whom and for how much.

```cpp
struct HitData {
	Agent* attacker = nullptr;
	int damage = 1;
	glm::vec2 direction = { 0.0f, 0.0f };
	bool parryable = true;
};

//Agent
virtual void HitEvent(const HitData&) {}
virtual void LandHitEvent(const HitData&) {}
```

The hurtbox builds the `HitData` from the hitbox's owner and the hitbox's `damage` and `parryable` properties, then calls `HitEvent` on its owner and `LandHitEvent` on the attacker. The old `TakeDamage()`, `TakeDamage(int)` and `OnHit()` on `Agent` are gone.

### 2. Input buffer

One small buffer replaces the dodge-only `dodgeBufferTimer` and covers light, heavy and dodge. A press is held for a window (start at 0.15 s, matching the dodge buffer today) and consumed when the player can act. Accepted, subject to tweaks.

```cpp
enum class CombatInput { Light, Heavy, Dodge };

class InputBuffer {
public:
	void Push(CombatInput input) { pending = input; timer = window; }
	void Update(float deltaTime) { if (timer > 0.0f) timer -= deltaTime; }
	std::optional<CombatInput> Consume();

private:
	std::optional<CombatInput> pending;
	float timer = 0.0f;
	float window = 0.15f;
};
```

One slot is enough for the demo: the newest press wins. A queue can come later if testing asks for it.

### 3. Combo builder

A combo is a tree: each node is an attack (an animator branch plus its hitbox frames), and its children say which input continues it and in which frame window. The builder is an editor panel next to the Animator editor that edits this tree and saves it through the existing registry, so combos are data, not code.

```cpp
struct ComboNode {
	std::string branch;
	int damage = 1;
	float cancelStart = 0.0f;
	float cancelEnd = 0.0f;
	int light = -1;
	int heavy = -1;
};

struct ComboData {
	std::vector<ComboNode> nodes;
	int lightRoot = 0;
	int heavyRoot = -1;
};
```

Children are indices into `nodes`, so the tree serializes flat. At runtime the player tracks the current node and its elapsed time; a buffered input inside the cancel window jumps to that child and sets the animator flag, and running past the window resets to the roots.

This depends on the open Animator gaps: a runtime hitbox pool and frame effects that actually toggle hitboxes. Those land in phase 2.

### 4. Weapons and swapping (phase 5)

A weapon holds its `ComboData`, its animation set and base damage. Most weapons need no code; a weapon with a special move (a charge shot, a dash strike) adds a `WeaponAbility` class picked by name.

Swapping (PR #5): mouse wheel up goes to the next slot, wheel down to the previous one, while playing. After a swap, further swaps are locked for `swapCooldown` (0.12 s); wheel ticks during the lock are dropped, not queued, so a fast flick moves exactly one slot. `weaponSlots` and `swapCooldown` are tunable in the inspector. While playing, the wheel no longer zooms the camera. Swapping mid-combo resets to the new weapon's roots.

Demo target: two weapons with distinct light and heavy strings, each three to four nodes deep.

## Talismans

Talismans take damage as a stack: a hit damages only the first unbroken talisman.

### Hook model (phase 1, PR #4)

| Hook | Fires | Use |
| --- | --- | --- |
| Passive | Always while equipped and unbroken | Stat changes folded into `modified` |
| While-Active | Only for the top unbroken talisman (the one the next hit breaks) | Stronger stat changes or on-hit effects that move down the stack |
| OnConsume | Once, the moment the talisman breaks | Burst effects: shockwave, heal the next talisman, brief invulnerability |
| HitEvent / LandHitEvent | When the player takes or lands a hit | Talismans reacting to combat |

Stat hooks are not per-frame calls. The player rebuilds `modified` whenever the stack changes (construction, `LoadFromInventory`, a break):

```cpp
class Talisman {
public:
	virtual void ApplyPassive(PlayerStats&) const {}
	virtual void ApplyWhileActive(PlayerStats&) const {}
	virtual void OnConsume(PlayerAgent&) {}
	virtual void HitEvent(PlayerAgent&, const HitData&) {}
	virtual void LandHitEvent(PlayerAgent&, const HitData&) {}
};

void PlayerAgent::RecalculateStats() {
	modified = base;

	for (const auto& talisman : talismans) {
		if (!talisman->GetIsBroken()) talisman->ApplyPassive(modified);
	}

	if (Talisman* active = GetActiveTalisman()) active->ApplyWhileActive(modified);
}
```

First users: `SpeedTalisman` (+10% move speed, Passive) and `MajorSpeedTalisman` (+25% move and dodge speed, While-Active). The numbers are placeholders.

### Remaining talisman work

1. Add one talisman per hook that shows it off, aiming for about six in the demo pool.
2. Show the descriptions (`ConstantPassiveDesc` and friends) on the EquipScreen hover.
3. Register tunable numbers (max HP, textures, effect values) so they show in an inspector.
4. Settle whether one unlock fills one slot (move, as built) or every slot (copy).

## Enemies (phase 4)

All five enemy types share one attack loop (approach, wind up, strike, recover), so the work is one reusable script plus per-enemy tuning. Today `BasicFollow` chases, and `BasicRanged` has an Orbiting/Retreating/Chasing/Attacking state machine whose Attacking state does nothing, logs every frame with `std::cout`, and moves by writing `position` directly instead of velocity.

### Shared attack loop

A windup is one sprite held for a set time, then the strike enables a hitbox (or spawns a projectile). Tinting or scaling that one sprite during the windup is the visual tell, which fits the few-assets goal.

```cpp
class EnemyAttack : public BehaviourScript {
protected:
	enum class Phase { Approach, Windup, Strike, Recover };

	virtual bool InRange() const = 0;
	virtual void Strike() = 0;

	Phase phase = Phase::Approach;
	float phaseTimer = 0.0f;
	float windup = 0.5f;
	float recover = 0.6f;
};
```

Windup and Recover are where the player gets punished or punishes, so they are serialized per enemy and tuned in the editor.

### Roster

| Enemy | Strike | Needs |
| --- | --- | --- |
| Melee | Short hitbox lunge | Shared loop, runtime hitbox pool |
| Ranged | Fires a projectile | Projectile agent with its own hitbox and team; finishes `BasicRanged` |
| Suicide bomber | Explodes in a radius, dies | Radius hitbox; death on strike |
| Elite | Two-hit string, one unparryable | `HitData.parryable`; chains two strikes |
| Boss | Three or four attack patterns, phase change at 50% HP | Pattern picker on top of the shared loop; enemy HP |

### Supporting work

- Enemy HP and death: `EnemyAgent` needs `HitEvent` with an HP pool and a death/despawn path.
- Hit feedback: flash and knockback on hit, reusing `ApplyKnockback`.
- Collider tunneling is still open; fast projectiles and dash attacks will show it, so a simple swept check belongs here.
- A spawner or arena script so the demo can run waves.

## Hit counter and parry (phase 3)

The hit counter is simple: `PlayerAgent::LandHitEvent` raises `HitLanded` for the Blessings.

Parry needs a different hitbox setup because, today, a hit is applied the moment a hitbox touches a hurtbox, in whatever order `PhysicsSystem` visits pairs. A parry has to win over the hurtbox in the same tick, so damage moves to a resolve step after collision. No parry code exists in git history, so this is a new design.

1. Add `ColliderType::Parry`. The player's parry box is a collider that the parry animation enables only during its active frames (through frame effects, which the Animator still needs).
2. During collision, hitbox overlaps with a hurtbox or a parry box are collected as contacts instead of applying damage.
3. After the collision loop, each hitbox activation resolves once per target: if the target's parry box is in the contacts and `HitData.parryable` is true, it is a parry; otherwise the hurtbox applies `HitEvent`.
4. A parry raises `Parry` for Blessings, staggers the attacker (forced Recover phase), and gives a short hit-stop for feel.
5. Each hitbox activation gets an id so one swing hits each target at most once, which the resolve step needs anyway.

Input: parry needs its own button (placeholder: L). Window to start with: 0.12 s active, 0.3 s recovery on a whiff, both serialized for tuning.

## Blessings (phase 6)

Blessings are one small system: each listens to player combat events, fills a condition, then runs an effect until a duration ends or the player is hit.

```cpp
enum class CombatEvent { HitLanded, HitTaken, Parry, Dodge, Kill };

class Blessing {
public:
	virtual ~Blessing() = default;
	virtual const char* GetType() const = 0;

	virtual void OnEvent(PlayerAgent&, CombatEvent, const HitData&) {}
	virtual void Update(PlayerAgent&, float) {}
	virtual bool OverridesMovement() const { return false; }
	virtual bool BlocksHit(const HitData&) { return false; }

protected:
	int charge = 0;
	int chargeNeeded = 0;
	float activeTimer = 0.0f;
};
```

`BlocksHit` lets a Blessing swallow a hit before talismans take it, which is how the invulnerability Blessing records negated damage. `OverridesMovement` hands control of movement to the active Blessing.

| Blessing | Condition | Effect | Ends |
| --- | --- | --- | --- |
| Shadowstep | 20 hits landed | Movement teleports the player behind the nearest enemy | Duration, or on hit |
| Retribution | N parries | Invulnerable for X seconds while summing negated damage, then explodes for that sum | After the explosion |
| A passive (TBD) | Active from run start | Stat boost or on-hit effect | On first hit taken |

## Editor tools

The rule everywhere: behaviour is code, numbers and art are data. This is the pattern components and scripts already use: a class is registered by type name in a factory, its tunable fields go through `RegisterProperties()`, and the inspector draws and saves them. The editor work per piece is a picker plus the existing property inspector, not a builder.

| Piece | Behaviour (code) | Tuned in the editor | Phase |
| --- | --- | --- | --- |
| Talismans | One class per talisman, overriding the hooks it needs | Max HP, textures, effect numbers | 1 |
| Animations | Animator runtime: frame effects toggle hitboxes and set flags | Branches, frames, transitions, hitbox frames | 2 |
| Attacks | Generic: an attack is an animation branch plus hitbox frames | Hitbox shape, damage, parryable; combo tree in the combo builder | 2 |
| Enemies | Shared attack script plus one script per enemy type | Windup, recover, range, speed, HP on the `.agent` prefab | 4 |
| Weapons | Generic weapon holding a combo tree; special moves as `WeaponAbility` classes | Combo tree, animation set, damage, the ability and its numbers | 5 |
| Blessings | One class per Blessing: its condition and its effect | Threshold, duration, effect values | 6 |

## Open questions

- [ ] Talisman Passive: does it keep applying after that talisman breaks, or only while unbroken (built as unbroken only)?
- [ ] Talisman unlocks: does one unlock fill one slot (move, as built) or every slot (copy)?
- [ ] Shadowstep: does taking a hit reset the 20-hit count, or only spending it?
- [ ] Retribution: how many parries, how many seconds of invulnerability, and does the explosion use raw negated damage or a multiplier?
- [ ] Parry button (L is a placeholder), and when controller input comes in.
- [ ] Third Blessing: which passive fills the demo slot?
