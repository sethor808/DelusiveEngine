#pragma once
#include <string>
#include <memory>
#include <vector>

class PropertyBase {
public:
    std::string name;
    virtual ~PropertyBase() = default;

    const std::string& GetName() const { return name; }

    virtual void Serialize(std::ostream& out) const = 0;
    virtual void Deserialize(std::istream& in) = 0;
    virtual void DrawImGui() = 0;
};

template<typename T>
class Property;

// ----------------------
// Registry
// ----------------------
class PropertyRegistry {
public:
    std::vector<std::unique_ptr<PropertyBase>> properties;

    PropertyRegistry() = default;
    PropertyRegistry(const PropertyRegistry&) = delete;
    PropertyRegistry& operator=(const PropertyRegistry&) = delete;
    PropertyRegistry(PropertyRegistry&&) noexcept = default;
    PropertyRegistry& operator=(PropertyRegistry&&) noexcept = default;

    template<typename T>
    void Register(const std::string& name, T* var);

    void Serialize(std::ostream& out) const;
    void Deserialize(std::istream& in);
    void DrawImGui();
};

#include "Property.inl"