#pragma once
#include <memory>
#include <vector>
#include <type_traits>
#include <string>
#include <fstream>
#include <DelusiveInternal/Components/TransformComponent.h>
#include <DelusiveInternal/Editor/EditorInterface.h>
#include <DelusiveInternal/Player/PlayerInputState.h>

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
    uint64_t GetID() const { return id; }
    void SetID(uint64_t newID) { id = newID; }

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
    T* GetComponentOfType();

    template<typename T>
    std::vector<T*> GetComponentsOfType();

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args);

    template<typename T>
    T* GetComponent() const;

    template<typename T>
    void RemoveComponent();

    Component* GetComponentByName(const std::string&);
    Component* GetComponentByID(uint64_t);
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
    uint64_t id = 0;

    bool editorMode = false;
    InteractionState interaction;

    std::vector<std::unique_ptr<Component>> components;

    std::string name;
    std::string type;
    uint64_t nextComponentID = 0;
    std::unique_ptr<PropertyRegistry> registry;

    void CloneBaseProperties(Agent*, Scene*) const;
};