#include <Delusive/Runtime/Core/UUIDManager.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/Scene/SceneSystem.h>
#include <cassert>

void UUIDManager::RegisterAgent(const UUID& id, Agent* agent)
{
    if (!id.IsValid() || agent == nullptr) return;

    std::lock_guard<std::mutex> lock(mutex_);
    agentRegistry_[id] = agent;
}

void UUIDManager::UnregisterAgent(const UUID& id)
{
    if (!id.IsValid()) return;

    std::lock_guard<std::mutex> lock(mutex_);
    agentRegistry_.erase(id);
}

Agent* UUIDManager::FindAgent(const UUID& id) const
{
    if (!id.IsValid()) return nullptr;

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = agentRegistry_.find(id);
    return it != agentRegistry_.end() ? it->second : nullptr;
}

bool UUIDManager::HasAgent(const UUID& id) const
{
    if (!id.IsValid()) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    return agentRegistry_.find(id) != agentRegistry_.end();
}

void UUIDManager::RegisterScene(const UUID& id, Scene* scene)
{
    if (!id.IsValid() || scene == nullptr) return;

    std::lock_guard<std::mutex> lock(mutex_);
    sceneRegistry_[id] = scene;
}

void UUIDManager::UnregisterScene(const UUID& id)
{
    if (!id.IsValid()) return;

    std::lock_guard<std::mutex> lock(mutex_);
    sceneRegistry_.erase(id);
}

Scene* UUIDManager::FindScene(const UUID& id) const
{
    if (!id.IsValid()) return nullptr;

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = sceneRegistry_.find(id);
    return it != sceneRegistry_.end() ? it->second : nullptr;
}

bool UUIDManager::HasScene(const UUID& id) const
{
    if (!id.IsValid()) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    return sceneRegistry_.find(id) != sceneRegistry_.end();
}

void UUIDManager::RegisterUIElement(const UUID& id, UIElement* element)
{
    if (!id.IsValid() || element == nullptr) return;

    std::lock_guard<std::mutex> lock(mutex_);
    uiRegistry_[id] = element;
}

void UUIDManager::UnregisterUIElement(const UUID& id)
{
    if (!id.IsValid()) return;

    std::lock_guard<std::mutex> lock(mutex_);
    uiRegistry_.erase(id);
}

UIElement* UUIDManager::FindUIElement(const UUID& id) const
{
    if (!id.IsValid()) return nullptr;

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = uiRegistry_.find(id);
    return it != uiRegistry_.end() ? it->second : nullptr;
}

bool UUIDManager::HasUIElement(const UUID& id) const
{
    if (!id.IsValid()) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    return uiRegistry_.find(id) != uiRegistry_.end();
}

void UUIDManager::RegisterSystem(const UUID& id, SceneSystem* system)
{
    if (!id.IsValid() || system == nullptr) return;

    std::lock_guard<std::mutex> lock(mutex_);
    systemRegistry_[id] = system;
}

void UUIDManager::UnregisterSystem(const UUID& id)
{
    if (!id.IsValid()) return;

    std::lock_guard<std::mutex> lock(mutex_);
    systemRegistry_.erase(id);
}

SceneSystem* UUIDManager::FindSystem(const UUID& id) const
{
    if (!id.IsValid()) return nullptr;

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = systemRegistry_.find(id);
    return it != systemRegistry_.end() ? it->second : nullptr;
}

bool UUIDManager::HasSystem(const UUID& id) const
{
    if (!id.IsValid()) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    return systemRegistry_.find(id) != systemRegistry_.end();
}

void UUIDManager::Clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    agentRegistry_.clear();
    sceneRegistry_.clear();
    uiRegistry_.clear();
    systemRegistry_.clear();
}