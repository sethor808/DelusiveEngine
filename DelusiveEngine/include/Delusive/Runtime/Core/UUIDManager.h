#pragma once
#include <Delusive/Runtime/Utils/UUID.h>
#include <unordered_map>
#include <mutex>

class Agent;
class Scene;
class UIElement;
class SceneSystem;

class UUIDManager {
public:
    // Non-copyable / non-movable
    UUIDManager() = default;
    UUIDManager(const UUIDManager&) = delete;
    UUIDManager& operator=(const UUIDManager&) = delete;
    UUIDManager(UUIDManager&&) = delete;
    UUIDManager& operator=(UUIDManager&&) = delete;

    // Agent registry
    void RegisterAgent(const UUID& id, Agent* agent);
    void UnregisterAgent(const UUID& id);
    Agent* FindAgent(const UUID& id) const;
    bool HasAgent(const UUID& id) const;

    // Scene registry
    void RegisterScene(const UUID& id, Scene* scene);
    void UnregisterScene(const UUID& id);
    Scene* FindScene(const UUID& id) const;
    bool HasScene(const UUID& id) const;

    // UIElement registry
    void RegisterUIElement(const UUID& id, UIElement* element);
    void UnregisterUIElement(const UUID& id);
    UIElement* FindUIElement(const UUID& id) const;
    bool HasUIElement(const UUID& id) const;

    // SceneSystem registry
    void RegisterSystem(const UUID& id, SceneSystem* system);
    void UnregisterSystem(const UUID& id);
    SceneSystem* FindSystem(const UUID& id) const;
    bool HasSystem(const UUID& id) const;

    // Utility
    void Clear();

private:
    mutable std::mutex mutex_;
    std::unordered_map<UUID, Agent*, UUID::Hash> agentRegistry_;
    std::unordered_map<UUID, Scene*, UUID::Hash> sceneRegistry_;
    std::unordered_map<UUID, UIElement*, UUID::Hash> uiRegistry_;
    std::unordered_map<UUID, SceneSystem*, UUID::Hash> systemRegistry_;
};