#pragma once
#include <memory>
#include <unordered_map>
#include <functional>
#include <string>
#include <DelusiveInternal/Talismans/Talisman.h>

class TalismanFactory {
public:
    using Creator = std::function<std::unique_ptr<Talisman>()>;

    void Register(const std::string& type, Creator creator) {
        creators[type] = std::move(creator);
    }

    std::unique_ptr<Talisman> Create(const std::string& type) const {
        auto it = creators.find(type);
        if (it == creators.find(type)) {
            return nullptr;
        }

        return it->second();
    }

    bool IsRegistered(const std::string& type) const {
        return creators.contains(type);
    }

private:
    std::unordered_map<std::string, Creator> creators;
};