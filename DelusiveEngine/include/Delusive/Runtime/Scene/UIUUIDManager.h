#pragma once
#include <Delusive/Runtime/Utils/UUID.h>
#include <unordered_map>

class UIElement; // forward declare

class UIUUIDManager {
public:
    void Register(UIElement* element, const UUID& id) {
        registry[id] = element;
    }

    void Unregister(const UUID& id) {
        registry.erase(id);
    }

    UIElement* Find(const UUID& id) const {
        if (auto it = registry.find(id); it != registry.end())
            return it->second;
        return nullptr;
    }

private:
    std::unordered_map<UUID, UIElement*, UUID::Hash> registry;
};