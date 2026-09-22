#pragma once
#include <Delusive/Runtime/Core/DelusiveParser.h>

template<typename T>
class Property;
class PropertyRegistry;
struct DelusiveInstance;

class PropertyBase {
public:
    std::string name;
    //Non-owning
    PropertyRegistry* registry = nullptr;
    virtual ~PropertyBase() = default;

    const std::string& GetName() const { return name; }

    virtual void Serialize(std::ostream& out) const = 0;
    virtual void Deserialize(std::istream& in) = 0;
    virtual void DrawImGui() = 0;

    //Second load phase: owned objects turn their parked recipe id into a live instance.
    //No-op for every property that does not own something.
    virtual void Resolve(DelusiveInstance&) {}

    //Save counterpart: owned objects contribute their own top level block.
    virtual void Collect(std::vector<DelusiveParser::DataBlock>&) const {}
};

// Registry
class PropertyRegistry {
public:
    std::string category = "";
    std::string type = "";

    std::vector<std::unique_ptr<PropertyBase>> properties;

    PropertyRegistry() = default;
    PropertyRegistry(const PropertyRegistry&) = delete;
    PropertyRegistry& operator=(const PropertyRegistry&) = delete;
    PropertyRegistry(PropertyRegistry&&) noexcept = default;
    PropertyRegistry& operator=(PropertyRegistry&&) noexcept = default;

    template<typename T>
    void Register(const std::string& name, T* var);

    void Serialize(std::ostream& out) const;
    //Fills a block instead of writing text - lets the caller add the header UUID
    void Serialize(DelusiveParser::DataBlock& out) const;
    //void Deserialize(std::istream& in);
    void Deserialize(const DelusiveParser::DataBlock&);
    void Resolve(DelusiveInstance&);
    void Collect(std::vector<DelusiveParser::DataBlock>&) const;
    void DrawImGui();
};

#include <Delusive/Runtime/Utils/Property.inl>