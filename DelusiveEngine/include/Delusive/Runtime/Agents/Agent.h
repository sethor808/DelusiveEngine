#pragma once
#include <memory>
#include <vector>
#include <type_traits>
#include <string>
#include <fstream>
#include <Delusive/Runtime/Components/TransformComponent.h>
#include <Delusive/Runtime/Editor/EditorInterface.h>
#include <Delusive/Runtime/Player/PlayerInputState.h>
#include <Delusive/Runtime/Utils/UUID.h>

class Component;
class PropertyRegistry;
class DelusiveRenderer;
class Collider;
class Scene;

class Agent {
public:
    explicit Agent(DelusiveRenderer&);
    Agent() = delete;
    virtual ~Agent();

    //Mandatory virtuals
    virtual std::unique_ptr<Agent> Clone(Scene*) const = 0;
    virtual std::string GetType() const = 0;

    //Base Gameplay hooks
    virtual void Update(float);
    virtual void Draw(const glm::mat4&) const; //Probably can be removed/changed

    //Serialization
    virtual void Serialize(std::ofstream&) const;
    virtual void Deserialize(std::ifstream&);

    //Editor
    virtual void DrawImGui();

    //Identity Handles
    UUID GetID() const { return id; }
    void SetID(UUID newID) { id = newID; }

    //Links
    virtual void LinkScene(Scene* scene) { sceneLink = scene; }
    Scene* GetScene() const { return sceneLink; }

    //Property managers
    virtual void RegisterProperties();
    void SetPosition(const glm::vec2&);
    void SetRotation(float);
    void SetScale(const glm::vec2&);
    void SetTransform(TransformComponent&);
    TransformComponent& GetTransform();
    TransformComponent& GetTransform() const;

    //Editing Tools
    void SetEditorMode(bool);
    void HandleMouse(const glm::vec2&, bool);

    // Component Handling
    void AddRawComponent(std::unique_ptr<Component> component);

    template<typename T>
    T* GetComponentOfType() {
        for (auto& comp : components) {
            if (T* casted = dynamic_cast<T*>(comp.get()))
                return casted;
        }
        return nullptr;
    }

    template<typename T>
    std::vector<T*> GetComponentsOfType() {
        static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

        std::vector<T*> result;

        for (auto& comp : components) {
            if (T* casted = dynamic_cast<T*>(comp.get())) {
                result.push_back(casted);
            }
        }

        return result;
    }

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value,
            "T must derive from Component");

        // Inject the renderer reference before forwarded args
        auto component = std::make_unique<T>(renderer, std::forward<Args>(args)...);
        component->SetOwner(this);
        if (!component->GetID().IsValid()) {
            component->SetID(UUID::GenerateRandom());
        }
        
        T* ptr = component.get();
        components.push_back(std::move(component));
        componentLookup[ptr->GetID()] = ptr;

        return ptr;
    }

    template<typename T, typename... Args>
    T* AddComponent(std::ifstream& in, Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value,
            "T must derive from Component");

        auto component = std::make_unique<T>(renderer, std::forward<Args>(args)...);

        T* ptr = component.get();
        components.push_back(std::move(component));
        if (ptr) {
            ptr->SetOwner(this);
            ptr->Deserialize(in);

            // Ensure component has a valid UUID after deserialization.
            // If the serialized data did not include an ID (or included an invalid one),
            // generate a new UUID so we don't end up with many zero-UUIDs.
            if (!ptr->GetID().IsValid()) {
                ptr->SetID(UUID::GenerateRandom());
            }

            componentLookup[ptr->GetID()] = ptr;
        }

        return ptr;
    }

    template<typename T>
    T* GetComponent() const {
        for (const auto& c : components) {
            if (auto ptr = dynamic_cast<T*>(c.get())) {
                return ptr;
            }
        }
        return nullptr;
    }

    template<typename T>
    void RemoveComponent() {
        components.erase(
            std::remove_if(
                components.begin(),
                components.end(),
                [](const std::unique_ptr<Component>& c) {
                    return dynamic_cast<T*>(c.get()) != nullptr;
                }
            ),
            components.end()
        );
    }

    Component* GetComponentByID(UUID);
    const std::vector<std::unique_ptr<Component>>& GetComponents() const;
    void RemoveComponentByPointer(Component*);

    //File I/O
    void SaveToFile(const std::string&) const;
    virtual void SaveToFile(std::ofstream&) const;
    void LoadFromFile(const std::string&);
    virtual void LoadFromFile(std::ifstream&);

    //Data
    void SetName(const std::string& n) { name = n; }
    const std::string& GetName() const { return name; }

    //Gameplay calls
    virtual void HandleInput(const PlayerInputState&) {}
    virtual void TakeDamage() {}
    virtual void TakeDamage(int) {}
    virtual void OnHit() {};

protected:
    TransformComponent transform;
    DelusiveRenderer& renderer;
    Scene* sceneLink = nullptr;
    //uint64_t id = 0; REMOVED FOR UUID SYSTEM
    UUID id;

    bool editorMode = false;
    InteractionState interaction;

    std::vector<std::unique_ptr<Component>> components;
    std::unordered_map<UUID, Component*, UUID::Hash> componentLookup;

    std::string name;
    std::string type;
    uint64_t nextComponentID = 0;
    std::unique_ptr<PropertyRegistry> registry;

    void CloneBaseProperties(Agent*, Scene*) const;
};