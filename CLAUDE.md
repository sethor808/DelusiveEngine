# DelusiveEngine code style

Write engine code the way the owner writes it. These rules come from the owner's own code (history up to the March 2026 "Editor Improvements" commit, before the serialization/animator overhaul). Where their code disagrees with itself, the rule below states which way to go; when touching a file, match it, and only normalize lines you are already changing.

Keep code minimal and tooling light. Prefer small, readable changes over new abstractions.

## Layout and files

- Engine headers live in `DelusiveEngine/include/Delusive/...`, sources mirror them in `DelusiveEngine/src/Delusive/...` (`Runtime/Agents/Agent.h` ↔ `Runtime/Agents/Agent.cpp`). One class per file, file named after the class.
- Game scripts: `DelusiveScripts/include/Scripts/<Group>/` and `DelusiveScripts/src/Scripts/<Group>/`.
- Every header starts with `#pragma once`. No include guards.
- Includes use angle brackets with the full path from the include root, never quotes or relative paths:
  ```cpp
  #include <Delusive/Runtime/Agents/Agent.h>
  #include <Delusive/Internal/Rendering/DelusiveRenderer.h>
  ```
- A `.cpp` includes its own header first, then other engine headers, then std/third-party.
- Grouping headers (`DelusiveComponents.h`, `DelusiveAgents.h`, `DelusiveSystems.h`) are for `.cpp` files that need many types; headers include only what they need.

## Headers and forward declarations

- Forward-declare anything only used by pointer or reference, in a block after the includes. Include only when the full type is needed (base classes, by-value members, inline bodies that call into the type).
  ```cpp
  #include <Delusive/Runtime/Agents/Agent.h>
  #include <Delusive/Runtime/Player/PlayerStats.h>

  class Talisman;
  class DelusiveInventory;
  ```
- A `//Forward declarations` label above the block is fine but optional.
- Parameter names are usually omitted in declarations when the type says it all; keep names where they add meaning (`float deltaTime`, `const glm::mat4& projection`):
  ```cpp
  void ApplyKnockback(const glm::vec2&, float);
  void TriggerInvul(float);
  ```

## Naming

- Types: `PascalCase`. Engine-wide types carry the `Delusive` prefix (`DelusiveRenderer`, `DelusiveRegistry`, `DelusiveTexture`). Families use a suffix or prefix: `PlayerAgent`/`EnemyAgent`, `SpriteComponent`/`ColliderComponent`, `UIButton`/`UICanvas`.
- Functions and methods: `PascalCase`, verb first. Common verbs: `Get`/`Set`, `Is`/`Has`, `Handle` (input/mouse), `Draw`, `Draw...ImGui` (editor panels), `Link` (non-owning back-references: `LinkScene`, `LinkCanvas`), `Fetch` (lookups that search: `FetchPlayer`), `Register`, `Clone`, `Serialize`/`Deserialize`.
- Members, locals, parameters: `camelCase`, no prefixes (no `m_`, no trailing `_`).
- Bools read as a state: `enabled`, `editorMode`, `dodging`, `toDelete`, `isDragging`, `wasDown`.
- Non-owning back pointers end in `Link` when they point "up" (`sceneLink`, `inventoryLink`); a plain name otherwise (`owner`, `gameManager`).
- Enums are `enum class` with `PascalCase` values: `ColliderType::Hurtbox`, `EditorMode::SceneEditor`.
- Macros and constants: `UPPER_SNAKE` in `Runtime/Utils/DelusiveMacros.h` (`DELUSIVE_PIXEL_SCALE`, `SCENE_PATH`, `ANIM_EXT`).
- Free-function groups go in a namespace named after the system: `namespace DelusiveParser`, `namespace DelusiveUI`, `namespace DelusiveEngine`. Classes are not namespaced. Never `using namespace`.
- Constructor parameters that would shadow a member get a leading underscore: `UIButton(DelusiveRenderer& _renderer) : UIElement(_renderer)`, `void SetName(const std::string& _name) { name = _name; }`.

## Class layout

- `public:` first, then `protected:`, then `private:`. Members (data) go at the bottom in `protected`/`private`; private helper methods go after the data.
- Order inside `public:`: constructors, deleted/defaulted special members, destructor, `Clone`/`GetType`, then grouped methods.
- Group methods under short `//Label` comments:
  ```cpp
  //Mandatory virtuals
  virtual std::unique_ptr<Agent> Clone(Scene*) const = 0;
  virtual std::string GetType() const = 0;

  //Serialization
  virtual void Serialize(std::ofstream&) const;
  virtual void Deserialize(std::ifstream&);
  ```
- Engine objects that hold a renderer reference delete the default constructor and copies, and default moves:
  ```cpp
  explicit Agent(DelusiveRenderer&);
  Agent() = delete;

  Component(const Component&) = delete;
  Component& operator=(const Component&) = delete;
  Component(Component&&) noexcept = default;
  Component& operator=(Component&&) noexcept = default;
  ```
- Copying goes through a virtual `Clone()` returning `std::unique_ptr<Base>`, built with `std::make_unique<Derived>(renderer)` and then field-by-field copies.
- Trivial getters/setters are inline one-liners in the header; anything longer goes in the `.cpp`.
- Overrides use `override` without repeating `virtual`.
- Default member values are given at the declaration: `bool enabled = true;`, `glm::vec2 velocity = { 0.0f, 0.0f };`, `Agent* owner = nullptr;`.

## Ownership and types

- Owning: `std::unique_ptr`, created with `std::make_unique`. No `shared_ptr`, no raw `new`/`delete`.
- Non-owning: raw pointers (`Scene*`, `Agent* owner`), defaulted to `nullptr`. Required, never-null dependencies are references (`DelusiveRenderer& renderer`).
- Collections of owned objects: `std::vector<std::unique_ptr<T>>`, with an `std::unordered_map<UUID, T*, UUID::Hash>` lookup next to it when ID lookup is needed.
- Pass `glm` vectors/matrices and strings by `const&`; pass scalars by value.
- `const` on getters and on any method that does not mutate.
- `auto` for iterators, `make_unique` results, and range-for (`for (auto& c : components)`, `for (const auto& child : children)`). Spell the type out for math and plain values (`glm::vec2 direction = ...`, `float distance = ...`).
- Float literals always carry `f`: `0.5f`, `1.0f`.
- Prefix increment in loops: `for (int i = 0; i < count; ++i)`.
- Prefer `static_cast` for new code.

## Formatting

- 4-space indentation (`.editorconfig` sets `indent_style = space`).
- Braces on the same line for functions, classes, control flow. Constructors with an initializer list put it on the next line and the brace on its own line:
  ```cpp
  ColliderComponent::ColliderComponent(DelusiveRenderer& renderer)
      : Component(renderer)
  {
      name = "New Collider";
      RegisterProperties();
  }
  ```
- `else` / `else if` start a new line after the closing brace:
  ```cpp
  if (currentHealth <= 0) {
      return -1;
  }
  else {
      return 0;
  }
  ```
- Guard clauses go on one line without braces: `if (!enabled) return;`, `if (!owner || !target) return;`. Multi-line bodies always get braces.
- `switch` cases sit at the same indent as the `switch`.
- Blank line between logical steps inside a function.

## Comments

- Light. Comments label sections or flag something unfinished; they don't narrate what the code does.
- No space after `//`, sentence-case, no trailing period: `//Mandatory virtuals`, `//Shallow copy`, `//Do custom copy`.
- Unfinished work: `//TODO: ` followed by the intent. `//TODO: Rip out interaction handling and move almost all of this into the editor itself`
- Things scheduled for removal are tagged on the line: `virtual void Deserialize(std::istream& in); //OLD TO BE REMOVED`
- Don't leave commented-out code blocks in new work.

## Logging and errors

- Log with `std::cout` for info and `std::cerr` for failures, prefixed with the class in brackets:
  ```cpp
  std::cerr << "[Animation] Failed to open file: " << path << std::endl;
  std::cout << "[ScriptComponent] Attached script: " << scriptContainer->scriptName << std::endl;
  ```
- No exceptions or asserts. Failures log and return a neutral value (`nullptr`, `false`, `0`) or fall back (`renderer.CreateFallbackWhiteTexture()`).
- Null-check links and lookups before use, usually as a guard clause.

## Engine patterns to reuse

- Editable/serialized fields are registered in `RegisterProperties()`, which calls the base first:
  ```cpp
  void UIButton::RegisterProperties() {
      UIElement::RegisterProperties();
      registry->Register("Label", &label);
      registry->Register("FontColor", &fontColor);
  }
  ```
- Base-class behaviour is called explicitly at the top of overrides (`BehaviourScript::Update(deltaTime);`) or the end of draw passes (`UIElement::Draw(projection);`).
- Rendering goes through `RenderCommand` submitted to `DelusiveRenderer`; don't issue GL calls from agents, components or UI elements.
- Asset paths come from the macros in `DelusiveMacros.h`; don't hard-code new path strings elsewhere.
